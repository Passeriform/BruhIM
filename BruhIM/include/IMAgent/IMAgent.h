#pragma once

#ifndef IMAGENT_H
#define IMAGENT_H

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#ifndef _WIN32
typedef int SOCKET;
#endif

#include "IMThreads/IMWorker.h"

static bool isInvalid(SOCKET sockfd) {
#ifdef _WIN32
	return sockfd == INVALID_SOCKET;
#else
	return sockfd < 0;
#endif
}

static bool isSocketError(SOCKET sockfd) {
#ifdef _WIN32
	return sockfd == SOCKET_ERROR;
#else
	return sockfd < 0;
#endif
}

class IMAgent {
protected:
	SOCKET sockfd;
	sockaddr_in address;

	IMWorker<int>* worker;

public:
	IMAgent();
	IMAgent(IMWorker<int>*);
	~IMAgent();
	int closeSocket();
};

#endif