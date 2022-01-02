#pragma once

#ifndef UTILITY_H
#define UTILITY_H

#include <libconfig.h++>

// Get error from socket error code.
std::string getErrorString(SOCKET);

// Specialized convenience method for fetching severity enum type value.
log::trivial::severity_level lookupLoggingSeverity(libconfig::Config*, log::trivial::severity_level);

// Boolean short-circuits for socket error-checking.
bool isInvalid(SOCKET);
bool isSocketError(SOCKET);

// Get peer details.
sockaddr_in getPeerAddress(SOCKET);
std::string getPeerNameAsPresentation(sockaddr_in);
std::string getPeerNameAsPresentation(SOCKET);
unsigned short getPeerPort(SOCKET);

// TODO: Find a way to compress these methods
// Convenience methods for handling global pending ACK states.
bool isAckPending(sockaddr_in);
int setAckPending(sockaddr_in);
int clearAckPending(sockaddr_in);
bool isAckPending(SOCKET);
int setAckPending(SOCKET);
int clearAckPending(SOCKET);

// TODO: Move to proper message utility
// Convenience method for chunking string.
std::vector<std::string> chunkify(std::string, int, const char*);

// Print socket details
int printSocketDetails(SOCKET);

// Convenience function for libconfig value lookup.
template <typename T>
T lookupConfig(std::string cfgKey, libconfig::Config* config, T defaultValue = T()) noexcept {
	try
	{
		const libconfig::Setting& cfgRoot = config->getRoot();

		T lVal;

		if (!cfgRoot.lookupValue(cfgKey, lVal)) {
			lVal = defaultValue;
			BOOST_LOG_TRIVIAL(error) << "Couldn't find setting " << cfgKey << ", defaulting to " << lVal;
		}

		return lVal;
	}
	catch (libconfig::SettingNotFoundException& nfex)
	{
		BOOST_LOG_TRIVIAL(error) << "Setting not found." << nfex.what();
		return defaultValue;
	}
}

// Convenience method to close a socket.
int closeSocket(SOCKET);

#endif