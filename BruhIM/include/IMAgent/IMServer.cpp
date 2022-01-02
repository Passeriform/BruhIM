#include <iostream>
#include <utility>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include "bruhEngine.h"
#include "IMServer.h"
#include "IMThreads/IMObjectPool.h"
#include "IMAgent/IMClient.h"
#include "IMException.h"
#include "Utility.h"
#include "../Constants.h"

namespace logging = boost::log;

int testBindPort(SOCKET sockfd, struct sockaddr_in server_addr, std::pair<int, int> portRange, int retries) {
	for (int port = portRange.first; port < portRange.second; port++) {
		int portAttempts = retries + 1;
		server_addr.sin_port = htons(port);

		std::cout << std::endl;
		BOOST_LOG_TRIVIAL(debug) << "Attempting to bind port : " << port;

		while (portAttempts--) {
			if (!isInvalid(::bind(
				sockfd,
				(struct sockaddr*)&server_addr,
				sizeof(server_addr)
			))) return port;

			BOOST_LOG_TRIVIAL(debug) << "Retrying port ("
				<< retries - portAttempts + 1
				<< "/"
				<< retries + 1
				<< ")";
		}
		std::cout << std::endl;
	}

	throw NoFreePortException("No free ports available.", __FILE__, __LINE__, __func__);
}

IMServer::IMServer()
try : IMAgent() {
	initializeWorkers();
}
catch (std::exception&) {
	std::throw_with_nested(ServerInitException("Server couldn't be initialized.", __FILE__, __LINE__, __func__));
}

IMServer::IMServer(SOCKET sockfd, sockaddr_in address)
try : IMAgent(sockfd, address) {
	initializeWorkers();
}
catch (std::exception&) {
	std::throw_with_nested(ServerInitException("Server couldn't be initialized.", __FILE__, __LINE__, __func__));
}

IMServer::~IMServer() {
	collectorWorker->markForDelete();
	listenerWorker->markForDelete();
}

int IMServer::initializeWorkers() {
	collectorWorker = IMObjectPool::addNewWorker();
	listenerWorker = IMObjectPool::addNewWorker();
	IMObjectPool::activateWorker(collectorWorker);
	IMObjectPool::activateWorker(listenerWorker);

	return 0;
}

int IMServer::prepare(int* port, int retries) {
	address.sin_family = AF_INET;
#ifdef _WIN32
	InetPton(AF_INET, (PCSTR)INADDR_ANY, &address.sin_addr);
#else
	inet_pton(AF_INET, INADDR_ANY, &address.sin_addr);
#endif

	try {
		*port = testBindPort(sockfd, address, std::make_pair(*port, *port + 500), retries);
		BOOST_LOG_TRIVIAL(debug) << "Port " << *port << " has been bound to server listener." << std::endl;
	}
	catch (std::exception& e) {
		BOOST_LOG_TRIVIAL(error) << "An exception was thrown while binding server port." << std::endl << getErrorString(sockfd) << std::endl << e.what() << std::endl;
		std::throw_with_nested(BindFailedException("Couldn't bind the socket.", __FILE__, __LINE__, __func__));
	}

	return 0;
}

int IMServer::acceptClients() {
	SOCKET acceptedSockfd = -1;

	while (true) {
		// TODO: Convert all C-style casts to reinterpret casts.
		// TODO: Use poll for accepting connections as well for non-blocking use.
		// TODO: Store address later as global list storages.
		if (isInvalid(acceptedSockfd = accept(sockfd, (struct sockaddr*)&address, NULL))) {
			BOOST_LOG_TRIVIAL(error) << "An exception was thrown while accepting connections on server." << std::endl << getErrorString(sockfd);
			// throw AcceptFailedException("Couldn't accept an incoming client connection.", __FILE__, __LINE__, __func__);
			continue;
		}

		char clientAddressPresentation[INET_ADDRSTRLEN];

#ifdef _WIN32
		InetNtop(AF_INET, &address.sin_addr, clientAddressPresentation, INET_ADDRSTRLEN);
#else
		inet_ntop(AF_INET, &address.sin_addr, clientAddressPresentation, INET_ADDRSTRLEN);
#endif

		// TODO: Replace with a printer utility.
		BOOST_LOG_TRIVIAL(debug) << "Incoming client connection" << std::endl << "Sockfd: " << acceptedSockfd << std::endl << "IP: " << clientAddressPresentation << std::endl << "Port: " << ntohs(address.sin_port);
		BOOST_LOG_TRIVIAL(debug) << "Connected to client at " << clientAddressPresentation << ":" << ntohs(address.sin_port);

		// TODO: Convert WSAPOLLDF struct instance to IMAgent wrapper.
		// IMAgent* acceptedClient = new IMAgent(acceptedSockfd, acceptedClientAddress);
		// connectedClients.push_back(acceptedClient);

		WSAPOLLFD acceptedClientPollfd = {
			acceptedSockfd,
			POLLRDNORM,
			0,
		};

		connectedClientPollfds.emplace_back(acceptedClientPollfd);

		BOOST_LOG_TRIVIAL(debug) << "Peer added to event listener's poll list.";

		if (isAckPending(address)) {
			clearAckPending(address);
		}
		else {
			setAckPending(address);

			BOOST_LOG_TRIVIAL(debug) << "Sending connection acknowledgement.";

			bruhEngine::client->connect(clientAddressPresentation, ntohs(address.sin_port), false);

			BOOST_LOG_TRIVIAL(debug) << "Cross connection established.";
		}

		// Send connection welcome message.
		// if (send(acceptedSockfd, Constants::WELCOME_MESSAGE, strlen(Constants::WELCOME_MESSAGE), 0) != strlen(Constants::WELCOME_MESSAGE)) {
		//	BOOST_LOG_TRIVIAL(error) << "An exception was thrown while sending acknowledgement message." << std::endl << getErrorString(sockfd);
		// 	throw UnstableAcceptException("Connection was established but couldn't send acknowledgement message.", __FILE__, __LINE__, __func__);
		// }
	}

	return 0;
}

int IMServer::listenEvents() {
	while (true) {

#ifdef _WIN32
		auto pollResult = WSAPoll(connectedClientPollfds.data(), (unsigned long)connectedClientPollfds.size(), 1);
#else
		auto pollResult = poll(&clientfd, 1, 1);
#endif

		if (pollResult) {
			for (auto clientfd = connectedClientPollfds.begin(); clientfd != connectedClientPollfds.end();) {
				try {
					// FIXME: Weird bug pointing to NULL.
					IMServer::handleInvalidPollStates(clientfd);
				}
				catch (std::exception& e) {
					BOOST_LOG_TRIVIAL(error) << "An exception was thrown while polling clients." << e.what();
					++clientfd;
					continue;
				}


				if (clientfd->revents & POLLRDNORM) {
					// TODO: Replace all newline calls like below with proper grouped statements printing.
					std::cout << std::endl;
					BOOST_LOG_TRIVIAL(debug) << "Got message from " << getPeerNameAsPresentation(clientfd->fd);

					int chunkCount = recvChunkCount((WSAPOLLFD*)&clientfd);

					std::string recvString;
					while (chunkCount--) {
						recvString.append(recvChunk(clientfd));
					}

					// TODO: Use message printer utility to print properly
					std::cout << "[ " << clientfd->fd << " ]" << std::setw(Constants::TAB) << recvString << std::endl;
				}
				++clientfd;
			}
		}
	}

	return 0;
}

int IMServer::recvChunkCount(WSAPOLLFD* pollSockfd) {
	int chunkCount;
	char* chunkCountBuffer = (char*)&chunkCount;
	if (isInvalid(recv(
		pollSockfd->fd,
		chunkCountBuffer,
		sizeof(uint32_t),
		0
	))) {
		BOOST_LOG_TRIVIAL(debug) << "Could not receive chunk count message from peer." << getPeerNameAsPresentation(pollSockfd->fd);
		throw SendMessageFailedException("Message couldn't be sent.", __FILE__, __LINE__, __func__);
	}

	chunkCount = ntohl(chunkCount);
	return chunkCount;
}

std::string IMServer::recvChunk(std::vector<WSAPOLLFD>::iterator pollSockfd) {
	char buffer[Constants::MAX_MESSAGE_SIZE];
	int bytesReceived = 0;
	bytesReceived = recv(pollSockfd->fd, buffer, Constants::MAX_MESSAGE_SIZE, 0);

	if (isSocketError(bytesReceived) || !bytesReceived) {
		BOOST_LOG_TRIVIAL(debug) << "Connection lost to " << pollSockfd->fd << getErrorString(bytesReceived);
		BOOST_LOG_TRIVIAL(info) << getPeerNameAsPresentation(pollSockfd->fd) << " disconnected...";

		connectedClientPollfds.erase(pollSockfd);
		::closeSocket(pollSockfd->fd);
		return "";
	}

	return std::string(buffer);
}

int IMServer::run() {
	collectorWorker->promote();
	listenerWorker->promote();
	try {
		if (isSocketError(listen(sockfd, Constants::MAX_CLIENTS)))
			throw ListenFailedException("Socket listening threw an exception.", __FILE__, __LINE__, __func__);

		BOOST_LOG_TRIVIAL(debug) << "The server socket started listening.";

		// Load workers on separate threads.
		collectorWorker->initializeThread<IMServer>(&IMServer::acceptClients, this);
		listenerWorker->initializeThread<IMServer>(&IMServer::listenEvents, this);
	}
	catch (std::exception&) {
		std::throw_with_nested(RunFailedException("Server couldn't be run due to an internal exception.", __FILE__, __LINE__, __func__));
	}

	return 0;
}

bool IMServer::handleInvalidPollStates(std::vector<WSAPOLLFD>::iterator sockIt) {
	try {
		switch (sockIt->revents) {
		case POLLERR:
			BOOST_LOG_TRIVIAL(error) << "Poll error occured on: " << getPeerNameAsPresentation(sockIt->fd) << getErrorString(sockIt->fd);
			BOOST_LOG_TRIVIAL(debug) << "Removing client from poll array.";
			throw SocketException("Socket encountered an error.", __FILE__, __LINE__, __func__);

		case POLLHUP:
			BOOST_LOG_TRIVIAL(error) << "Poll hangup occured on: " << getPeerNameAsPresentation(sockIt->fd) << getErrorString(sockIt->fd);
			BOOST_LOG_TRIVIAL(debug) << "Removing client from poll array.";
			throw SocketHangupException("Socket was hanged up by the peer.", __FILE__, __LINE__, __func__);

		case POLLNVAL:
			BOOST_LOG_TRIVIAL(error) << "Poll invalid socket used on: " << getPeerNameAsPresentation(sockIt->fd) << getErrorString(sockIt->fd);
			BOOST_LOG_TRIVIAL(debug) << "Removing client from poll array.";
			throw InvalidSocketException("Invalid socket was encountered.", __FILE__, __LINE__, __func__);
		default:
			return 1;
		}
	}
	catch (std::exception& e) {
		if (sockIt != connectedClientPollfds.end()) {
			auto erasedSocket = connectedClientPollfds.erase(sockIt);
			::closeSocket((SOCKET)erasedSocket->fd);
		}
		throw e;
	}
}
