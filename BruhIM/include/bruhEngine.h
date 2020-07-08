#ifndef BRUHENGINE_H
#define BRUHENGINE_H

#include <libconfig.h++>

namespace bruhEngine {
	int init(libconfig::Config *);
	int runServer(libconfig::Config *);
	int attachDaemon(int);
	int runDaemon(libconfig::Config *);

	template<typename T>
	T lookupConfig(std::string, libconfig::Config *);
}

#include <bruhEngine.tpp>

#endif