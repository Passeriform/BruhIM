#pragma once

#ifndef IMCLIENT_H
#define IMCLIENT_H

#include "IMAgent.h"
#include "IMThreads/IMWorker.h"

class IMClient : public IMAgent {
	SOCKET sockfd;
	sockaddr_in address;

	IMWorker<int>* worker;

public:
	IMClient();
	int connect(std::string, int);
	int sendMessage(std::string);
	int joinRoom(std::string[]);
	int leaveRoom(std::string[]);
	int disconnect(std::string[]);
};

#endif