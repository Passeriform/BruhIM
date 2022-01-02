#pragma once

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Constants {
	constexpr auto MAX_MESSAGE_SIZE = 16;
	constexpr auto MAX_WORKERS = 20;
	constexpr auto MAX_CLIENTS = 40;
	constexpr auto WELCOME_MESSAGE = "[ACK] Connected to server";
	constexpr auto SELF_ADDRESS = "127.0.0.1";
	constexpr auto DEFAULT_PORT = 3000;
	constexpr auto MAX_PORT_RETRIES = 3;
	constexpr auto TAB = 15;
}

#endif