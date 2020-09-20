#pragma once

#ifndef BRUHENGINE_H
#define BRUHENGINE_H

#include <libconfig.h++>

#include "IMThreads/IMWorker.h"
#include "IMAgent/IMServer.h"
#include "IMAgent/IMClient.h"

namespace bruhEngine {
	extern IMServer* server;
	extern IMClient* client;

	int init(libconfig::Config*);
	int runServer(libconfig::Config*);
	int attachDaemon(int);
	int runDaemon(libconfig::Config*);
	int streamInput();
	int wrapUp();

	template <typename T>
	T lookupConfig(std::string cfgKey, libconfig::Config* config, T defaultValue = T()) {
		try
		{
			const libconfig::Setting& cfgRoot = config->getRoot();

			T lVal;

			if (!cfgRoot.lookupValue(cfgKey, lVal)) {
				lVal = defaultValue;
				std::cerr << "Couldn't find setting " << cfgKey << ", defaulting to " << lVal << "\n";
			}

			return lVal;
		}
		catch (const libconfig::SettingNotFoundException& nfex)
		{
			// Ignore.
			return T();
		}
	}
}

#endif