#pragma once

#ifndef BRUHENGINE_H
#define BRUHENGINE_H

#include <libconfig.h++>

#include "IMThreads/IMWorker.h"
#include "IMAgent/IMServer.h"
#include "IMAgent/IMClient.h"

namespace bruhEngine {
	/// Server micro-app reference
	extern IMServer* server;

	/// Client micro-app reference
	extern IMClient* client;

	// Globally accessible pending acknowledgement list.
	extern std::vector<struct sockaddr_in> pendingConnAck;

	// Initialize configurations. Primed to run a server/client.
	int init(libconfig::Config*);
	// Run active server and enable listeners.
	int runServer(libconfig::Config*);
	// Reattach daemon to active window.
	int attachDaemon(int);
	// Run server in background.
	int runDaemon(libconfig::Config*);
	// Wrap and cleanup.
	int wrapUp();

	// Convenience method for setting up keyboard input
	int streamInput() noexcept;
}

#endif