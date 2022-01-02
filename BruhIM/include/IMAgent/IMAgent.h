#pragma once

#ifndef IMAGENT_H
#define IMAGENT_H

#ifdef _WIN32
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

class IMAgent {
protected:
	// IM agent properties.
	SOCKET sockfd;
	sockaddr_in address;

public:
	// Default agent.
	IMAgent();
	// Parameterized IM agent properties.
	IMAgent(SOCKET, sockaddr_in);
	// Destroy agent.
	~IMAgent();

	// Convenience method to close socket.
	int closeSocket();
};

#endif