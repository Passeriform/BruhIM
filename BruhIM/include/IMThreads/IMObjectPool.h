#pragma once

#ifndef IMOBJECTPOOL_H
#define IMOBJECTPOOL_H

#include <list>
#include <string>

// TODO: Add resource provider facade with list of always active worker instances.
#include "IMThreads/IMWorker.h"

class IMObjectPool {
	// Pool current worker count.
	static int workerCount;

	// TODO: Change this structure to a flag for active/idle state.
	static std::list<IMWorker<int>*> busyWorkers;
	static std::list<IMWorker<int>*> idleWorkers;

public:
	// Pick a worker resource from the pool and use as new.
	// TODO: Rename to requestWorker
	static IMWorker<int>* addNewWorker();
	// Set worker to active state.
	static void activateWorker(IMWorker<int>*);
	// Flush worker instance. Set state to idle.
	static void deleteWorker(IMWorker<int>*);
};

#endif