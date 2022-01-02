#pragma once

#ifndef IMCLIENT_H
#define IMCLIENT_H

#include "IMAgent.h"
#include "IMThreads/IMWorker.h"
#include "../Constants.h"

class IMClient : public IMAgent {
public:
	// Initialize IM client.
	IMClient();

	// Connect to peer.
	int connect(std::string, int = Constants::DEFAULT_PORT, bool = true);
	// Send message to active peer/room.
	int sendMessage(std::string);
	// Create room.
	int createRoom(std::string[]);
	// Join room.
	int joinRoom(std::string[]);
	// Leave room.
	int leaveRoom(std::string[]);
	// Disconnect a peer.
	int disconnect(std::string[]);
	// Print client status.
	int printStatus();
};

#endif