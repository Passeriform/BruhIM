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

#include "IMAgent.h"
#include "IMThreads/IMObjectPool.h"
#include "IMException.h"
#include "Utility.h"
#include "../Constants.h"

namespace logging = boost::log;

IMAgent::IMAgent() {
#ifdef _WIN32
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(1, 1), &wsa_data);
#endif

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (isInvalid(sockfd)) {
		throw SocketNotCreatedException("Socket couldn't be created.", __FILE__, __LINE__, __func__);
	}

	IN_ADDR defaultInetAddr;

#ifdef _WIN32
	InetPton(AF_INET, (PCSTR)Constants::SELF_ADDRESS, &defaultInetAddr);
#else
	inet_pton(AF_INET, (PCSTR)Constants::SELF_ADDRESS, &defaultInetAddr);
	inet_addr defaultInetAddr = inet_addr(Constants::SELF_ADDRESS);
#endif

	address = {
		AF_INET,
		htons(Constants::DEFAULT_PORT),
		{
			defaultInetAddr
		},
	};
}

IMAgent::IMAgent(SOCKET sockfd, sockaddr_in address) {
	this->sockfd = sockfd;
	this->address = address;
}

IMAgent::~IMAgent() {
#ifdef _WIN32
	WSACleanup();
#endif

	closeSocket();
}

int IMAgent::closeSocket() {
	return ::closeSocket(sockfd);
}