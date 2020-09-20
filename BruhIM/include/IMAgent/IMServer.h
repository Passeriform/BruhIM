#pragma once

#ifndef IMSERVER_H
#define IMSERVER_H

#include <vector>

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

#include "IMAgent.h"
#include "IMClient.h"
#include "IMThreads/IMWorker.h"

class IMServer : public IMAgent {
	std::vector<IMAgent*> connectedClients;

	IMWorker<int>* worker;

public:
	IMServer();
	IMServer(IMWorker<int>*);
	int prepareSocket(int, int);
	int processClients();
	int setup();
	int setup(int);
	int run();
	IMWorker<int>* getWorker();
};

#endif