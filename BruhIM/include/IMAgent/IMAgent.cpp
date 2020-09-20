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

#include "IMAgent.h"
#include "IMException.h"

namespace logging = boost::log;

IMAgent::IMAgent() {
#ifdef _WIN32
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(1, 1), &wsa_data);
#endif

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (isInvalid(sockfd))
		throw SocketNotCreatedException("Socket couldn't be created.", __FILE__, __LINE__, __func__);
}


IMAgent::IMAgent(IMWorker<int>* workerPtr) : IMAgent() {
	worker = workerPtr;
}

IMAgent::~IMAgent() {
#ifdef _WIN32
	WSACleanup();
#endif

	closeSocket();
}

int IMAgent::closeSocket() {
	int status = 0;

#ifdef _WIN32
	status = shutdown(sockfd, SD_BOTH);
	if (status == 0) { status = closesocket(sockfd); }
#else
	status = shutdown(sockfd, SHUT_RDWR);
	if (status == 0) { status = close(sockfd); }
#endif

	return status;
}