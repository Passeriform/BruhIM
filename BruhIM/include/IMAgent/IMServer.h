#pragma once

#ifndef IMSERVER_H
#define IMSERVER_H

#include <vector>

#ifdef _WIN32
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
#include "IMAgent.h"
#include "IMThreads/IMWorker.h"
#include "../Constants.h"

class IMServer : public IMAgent {
	// TODO: Use shared_ptr and make data sharing explicit.
	// Assigned IM worker for collecting client accept requests.
	IMWorker<int>* collectorWorker;
	// Assigned IM worker for client event listen.
	IMWorker<int>* listenerWorker;

	// Connected client listener references.
	std::vector<WSAPOLLFD> connectedClientPollfds;

public:
	// Initialize default server.
	IMServer();
	// Parametrized worker instance.
	IMServer(SOCKET, sockaddr_in);
	// Destroy IM server.
	~IMServer();

	// Setting up configuration for server.
	int prepare(int*, int = Constants::MAX_PORT_RETRIES);
	// Worker method for accepting clients.
	int acceptClients();
	// Worker method to listen to events on accepted clients.
	int listenEvents();
	// Run server on a thread.
	int run();

	// Convenience method for initializing IM workers.
	int initializeWorkers();

	// Convenience method for receiving messages.
	int recvChunkCount(WSAPOLLFD*);
	std::string recvChunk(std::vector<WSAPOLLFD>::iterator);

	// Invalid socket poll handler
	bool handleInvalidPollStates(std::vector<WSAPOLLFD>::iterator);

	// Status print friend.
	friend int IMClient::printStatus();
};

#endif