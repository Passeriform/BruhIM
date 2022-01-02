#include <iostream>
#include <boost/assign/list_of.hpp>
#include <boost/bimap.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <libconfig.h++>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include "bruhEngine.h"
#include "IMException.h"
#include "Utility.h"
#include "../constants.h"

std::string getErrorString(SOCKET sockfd) {
#ifdef _WIN32
	return std::to_string(WSAGetLastError());
#endif
	int error = 0;
	int errorLength = sizeof(error);
	int getOptStatus = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&error, &errorLength);

	return strerror(getOptStatus || error);
}

// TODO: Convert convenience methods below to return a class with all peer details instead.
// TODO: Add method to address and port strings and use as getPeerDetails(sockfd).port.asNetwork() / getPeerDetails(sockfd).port.asPresentation()
sockaddr_in getPeerAddress(SOCKET sockfd) {
	struct sockaddr_in address;
	socklen_t length = sizeof(sockaddr_in);

	if (isInvalid(getpeername(sockfd, (struct sockaddr*)&address, &length))) {
		throw InvalidSocketException("Could not fetch peer name from socket", __FILE__, __LINE__, __func__);
	}

	return address;
}

std::string getPeerNameAsPresentation(SOCKET sockfd) {
	auto address = getPeerAddress(sockfd);
	return getPeerNameAsPresentation(address);
}

std::string getPeerNameAsPresentation(sockaddr_in address) {
	char ntop[INET_ADDRSTRLEN];

#ifdef _WIN32
	InetNtop(AF_INET, &address.sin_addr, ntop, INET_ADDRSTRLEN);
#else
	inet_ntop(AF_INET, &address.sin_addr, ntop, INET_ADDRSTRLEN);
#endif

	return std::string(ntop);
}

unsigned short getPeerPort(SOCKET sockfd) {
	auto address = getPeerAddress(sockfd);

	return address.sin_port;
}

std::vector<std::string> chunkify(std::string message, int chunkSize, const char* packingChar) {
	std::vector<std::string> chunks;
	int chunkCount = (int)ceil(message.size() / ((float)(chunkSize - strlen(packingChar))));
	int unpaddedChunkSize = chunkSize - strlen(packingChar);

	for (auto chunkIdx = 0; chunkIdx < chunkCount; ++chunkIdx) {
		std::string chunk = message.substr(chunkIdx * unpaddedChunkSize, unpaddedChunkSize);
		chunk += packingChar;
		chunks.push_back(chunk);
	}

	return chunks;
}

log::trivial::severity_level lookupLoggingSeverity(libconfig::Config* config, log::trivial::severity_level defaultLoggingLevel) {
	typedef boost::bimap<log::trivial::severity_level, std::string> severityMapType;
	const severityMapType severityBMap = boost::assign::list_of<severityMapType::relation>
		(log::trivial::debug, "debug")
		(log::trivial::trace, "trace")
		(log::trivial::info, "info")
		(log::trivial::warning, "warning")
		(log::trivial::error, "error")
		(log::trivial::fatal, "fatal");

	// TODO: Change default back to info.
	std::string logLevelString = lookupConfig<std::string>("logLevel", config, "debug");
	auto itt = severityBMap.right.find(logLevelString);
	return itt->second;
}

bool isInvalid(SOCKET sockfd) {
#ifdef _WIN32
	return sockfd == INVALID_SOCKET;
#else
	return sockfd < 0;
#endif
}

bool isSocketError(SOCKET sockfd) {
#ifdef _WIN32
	return sockfd == SOCKET_ERROR;
#else
	return sockfd < 0;
#endif
}

int setAckPending(SOCKET sockfd) {
	auto address = getPeerAddress(sockfd);
	return setAckPending(address);
}

bool isAckPending(SOCKET sockfd) {
	auto address = getPeerAddress(sockfd);
	return isAckPending(address);
}

int clearAckPending(SOCKET sockfd) {
	auto address = getPeerAddress(sockfd);
	return clearAckPending(address);
}

int setAckPending(sockaddr_in address) {
	bruhEngine::pendingConnAck.emplace_back(address);
	return 1;
}

bool isAckPending(sockaddr_in searchAddress) {
	return std::find_if(bruhEngine::pendingConnAck.begin(), bruhEngine::pendingConnAck.end(), [&searchAddress](const sockaddr_in& currAddress) {
		return currAddress.sin_addr.s_addr == searchAddress.sin_addr.s_addr && currAddress.sin_port == searchAddress.sin_port;
	}) != bruhEngine::pendingConnAck.end();
}

int clearAckPending(sockaddr_in searchAddress) {
	std::remove_if(bruhEngine::pendingConnAck.begin(), bruhEngine::pendingConnAck.end(), [&searchAddress](const sockaddr_in& currAddress) {
		return currAddress.sin_addr.s_addr == searchAddress.sin_addr.s_addr && currAddress.sin_port == searchAddress.sin_port;
	});

	return 1;
}

int printSocketDetails(SOCKET sockfd) {
	std::cout << std::endl;
	BOOST_LOG_TRIVIAL(info) << "FD:" << std::setw(Constants::TAB) << sockfd;
	BOOST_LOG_TRIVIAL(info) << "Address:" << std::setw(Constants::TAB) << getPeerNameAsPresentation(sockfd);
	BOOST_LOG_TRIVIAL(info) << "Port:" << std::setw(Constants::TAB) << getPeerPort(sockfd);
	std::cout << std::endl;
	return 0;
}

int closeSocket(SOCKET sockfd) {
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