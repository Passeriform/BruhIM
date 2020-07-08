#include <iostream>

#include <libconfig.h++>

#include <bruhEngine.h>
#include <IMThreads/IMObjectPool.h>

using namespace libconfig;

int bruhEngine::init(libconfig::Config * config) {
    try
    {
        config->readFile("config/engine.cfg");
    }
    catch (const FileIOException& fioex)
    {
        std::cerr << "I/O error while reading file." << fioex.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const ParseException& pex)
    {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
            << " - " << pex.getError() << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}

int bruhEngine::runServer(libconfig::Config * config) {
    int port = lookupConfig<int>("port", config);
	
    IMObjectPool::assignWorker(port);
	return 0;
}

int bruhEngine::attachDaemon(int threadObjectId) {
	return 0;
}

int bruhEngine::runDaemon(libconfig::Config * config) {
	int threadObjectId = runServer(config);
	attachDaemon(threadObjectId);
	return 0;
}
