#include <cmath>
#include <numeric>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#ifdef _WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include "bruhEngine.h"
#include "IMClient.h"
#include "IMException.h"
#include "Utility.h"
#include "../Constants.h"

namespace logging = boost::log;

IMClient::IMClient()
try : IMAgent() { }
catch (std::exception&) {
	std::throw_with_nested(ClientInitException("Client couldn't be initialized.", __FILE__, __LINE__, __func__));
}

int IMClient::connect(std::string addressString, int port, bool setAgentSocketOnSuccess) {
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);

	address.sin_family = AF_INET;
	address.sin_port = htons(port);
#ifdef _WIN32
	InetPton(AF_INET, (PCSTR)addressString.c_str(), &address.sin_addr);
#else
	address.sin_addr.s_addr = inet_addr(addressString.c_str());
#endif

	if (isInvalid(::connect(
		sockfd,
		(struct sockaddr*)&address,
		sizeof(address)
	))) {
		BOOST_LOG_TRIVIAL(error) << "An exception was thrown while connecting to peer." << std::endl << getErrorString(sockfd);
		throw ConnectionFailedException("Could not connect to server.", __FILE__, __LINE__, __func__);
	}

	if (setAgentSocketOnSuccess) {
		this->sockfd = sockfd;
	}

	return sockfd;
}

// TODO: Add contact list and connected clients state. Send message to recently connected contact.
int IMClient::sendMessage(std::string message) {
	auto chunks = chunkify(message, Constants::MAX_MESSAGE_SIZE, "");
	auto chunkCount = chunks.size();

	// Chunk count to be transported before message.
	uint32_t chunkCountNetwork = htonl(chunkCount);

	// Implement contact list with recent contact picking mechanism to select in send.
	if (isInvalid(send(
		sockfd,
		(const char*)&chunkCountNetwork,
		sizeof(chunkCountNetwork), 0)
	)) {
		BOOST_LOG_TRIVIAL(error) << "An exception was thrown while sending chunk count message." << std::endl << getErrorString(sockfd);
		throw SendMessageFailedException("Message couldn't be sent.", __FILE__, __LINE__, __func__);
	}

	for (auto it = 0; it < chunkCount; ++it) {
		BOOST_LOG_TRIVIAL(debug) << "Sending chunk: " << chunks[it] << " (count: " << it + 1 << ", size: " << chunks[it].size() << ")";

		if (isInvalid(send(
			sockfd,
			chunks[it].data(),
			chunks[it].size(), 0)
		)) {
			BOOST_LOG_TRIVIAL(error) << "An exception was thrown while sending message chunk." << std::endl << getErrorString(sockfd);
			throw SendMessageFailedException("Message couldn't be sent.", __FILE__, __LINE__, __func__);
		}
	}

	return 0;
}

int IMClient::createRoom(std::string[]) {
	return 0;
}

int IMClient::joinRoom(std::string args[]) {
	return 0;
}

int IMClient::leaveRoom(std::string args[]) {
	return 0;
}

int IMClient::disconnect(std::string args[]) {
	return 0;
}

int IMClient::printStatus() {
	BOOST_LOG_TRIVIAL(info) << "Pending ACK hosts:";
	std::for_each(
		bruhEngine::pendingConnAck.begin(),
		bruhEngine::pendingConnAck.end(),
		[](sockaddr_in sockAddress) {
		BOOST_LOG_TRIVIAL(info) << getPeerNameAsPresentation(sockAddress) << std::setw(Constants::TAB) << ntohs(sockAddress.sin_port);
	}
	);

	BOOST_LOG_TRIVIAL(info) << "Active Listeners:";
	std::for_each(
		bruhEngine::server->connectedClientPollfds.begin(),
		bruhEngine::server->connectedClientPollfds.end(),
		[](WSAPOLLFD pollEntry) { printSocketDetails(pollEntry.fd); }
	);
	return 0;
}