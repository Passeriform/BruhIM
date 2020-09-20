#include <iostream>
#include <utility>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include "IMServer.h"
#include "IMAgent/IMClient.h"
#include "IMException.h"
#include "../constants.h"

namespace logging = boost::log;

int testBindPort(SOCKET sockfd, struct sockaddr_in server_addr, std::pair<int, int> portRange, int retries) {
	for (int port = portRange.first; port < portRange.second; port++) {
		int portAttempts = retries + 1;
		server_addr.sin_port = htons(port);

		BOOST_LOG_TRIVIAL(info) << "Attempting to bind port : " << port;

		while (portAttempts--) {
			if (!isInvalid(::bind(
				sockfd,
				(struct sockaddr*)&server_addr,
				sizeof(server_addr)
			))) return port;

			BOOST_LOG_TRIVIAL(info) << "Retrying port ("
				<< retries - portAttempts + 1
				<< "/"
				<< retries + 1
				<< ")";
		}
	}

	throw NoFreePortException("No free ports available.", __FILE__, __LINE__, __func__);
}

IMServer::IMServer()
try : IMAgent() { }
catch (std::exception& e) {
	std::throw_with_nested(ServerInitException("Server couldn't be initialized.", __FILE__, __LINE__, __func__));
}

IMServer::IMServer(IMWorker<int>* workerPtr)
try : IMAgent() {
	worker = workerPtr;
}
catch (std::exception& e) {
	std::throw_with_nested(ServerInitException("Server couldn't be initialized.", __FILE__, __LINE__, __func__));
}

int IMServer::prepareSocket(int port = 3000, int retries = 3) {
	address.sin_family = AF_INET;
#ifdef _WIN32
	InetPton(AF_INET, (PCSTR)(INADDR_ANY), &address.sin_addr.s_addr);
#else
	address.sin_addr.s_addr = inet_addr("192.168.1.4");
#endif

	try {
		port = testBindPort(sockfd, address, std::make_pair(port, port + 500), retries);
	}
	catch (std::exception& e) {
		std::throw_with_nested(BindFailedException("Couldn't bind the socket.", __FILE__, __LINE__, __func__));
	}

	BOOST_LOG_TRIVIAL(info) << "Port Binding Successful.";

	if (isSocketError(listen(sockfd, constants::MAX_CLIENTS)))
		throw ListenFailedException("Socket listening threw an exception.", __FILE__, __LINE__, __func__);

	BOOST_LOG_TRIVIAL(info) << "The server socket started listening.";

	return 0;
}

int IMServer::processClients() {
	struct sockaddr_in clientAddress;
	SOCKET clientSockfd;

	while (1) {
		clientSockfd = -1;

		while (isInvalid(clientSockfd)) {
			clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddress, NULL);
		}

		throw AcceptFailedException("Couldn't accept an incoming client connection.", __FILE__, __LINE__, __func__);
	}

	return 0;
}

int IMServer::setup() {
	try {
		prepareSocket();
	}
	catch (std::exception& e) {
		std::throw_with_nested(SetupFailedException("Setup failed due to an internal exception.", __FILE__, __LINE__, __func__));
	}

	return 0;
}

int IMServer::setup(int port) {
	try {
		prepareSocket(port);
	}
	catch (std::exception& e) {
		std::throw_with_nested(SetupFailedException("Setup failed due to an internal exception.", __FILE__, __LINE__, __func__));
	}

	return 0;
}

int IMServer::run() {
	try {
		worker->initializeThread<IMServer>(&IMServer::processClients, this);
	}
	catch (std::exception& e) {
		std::throw_with_nested(RunFailedException("Server couldn't be run due to an internal exception.", __FILE__, __LINE__, __func__));
	}

	return 0;
}

IMWorker<int>* IMServer::getWorker() {
	return worker;
}
