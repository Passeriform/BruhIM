#include <cmath>
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

#include "IMClient.h"
#include "IMException.h"
#include "../constants.h"

namespace logging = boost::log;

IMClient::IMClient()
try : IMAgent() { }
catch (std::exception& e) {
	std::throw_with_nested(ClientInitException("Client couldn't be initialized.", __FILE__, __LINE__, __func__));
}

int IMClient::connect(std::string addressString = "192.168.1.4", int port = 3000) {
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
#ifdef _WIN32
	InetPton(AF_INET, (PCSTR)addressString.c_str(), &address.sin_addr.s_addr);
#else
	address.sin_addr.s_addr = inet_addr(addressString.c_str());
#endif

	if (isInvalid(::connect(
		sockfd,
		(struct sockaddr*)&address,
		sizeof(address)
	))) {
		int error = 0;
		socklen_t len = sizeof(error);
		int retval = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&error, &len);

		if (retval != 0) {
			fprintf(stderr, "error getting socket error code: %s\n", strerror(retval));
		}

		if (error != 0) {
			fprintf(stderr, "socket error: %s\n", strerror(error));
		}

		throw ConnectionFailedException("Could not connect to server.", __FILE__, __LINE__, __func__);
	}

	return 0;
}

int IMClient::sendMessage(std::string message) {
	int origLength = message.size();
	int finalLength = constants::MAX_MESSAGE_SIZE * (int)ceil(message.size() / (float)constants::MAX_MESSAGE_SIZE);

	uint32_t chunkCount = htonl(origLength / constants::MAX_MESSAGE_SIZE);

	char* chunkCountChar = (char*)&chunkCount;

	message = message + std::string(finalLength - origLength, '\0');

	if (isInvalid(send(
		sockfd,
		chunkCountChar,
		sizeof(chunkCount), 0)
	)) throw SendMessageFailedException("Message couldn't be sent.", __FILE__, __LINE__, __func__);

	for (auto it = 0; it < chunkCount; it++) {
		if (isInvalid(send(
			sockfd,
			message.substr((it * constants::MAX_MESSAGE_SIZE), constants::MAX_MESSAGE_SIZE).c_str(),
			sizeof(constants::MAX_MESSAGE_SIZE), 0)
		)) throw SendMessageFailedException("Message couldn't be sent.", __FILE__, __LINE__, __func__);
	}

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
