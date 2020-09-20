#pragma once

#ifndef IMOBJECTPOOL_H
#define IMOBJECTPOOL_H

#include <list>
#include <string>

#include "IMThreads/IMWorker.h"

class IMObjectPool {
	static int workerCount;
	static std::list<IMWorker<int>*> busyWorkers;
	static std::list<IMWorker<int>*> idleWorkers;

public:
	static IMWorker<int>* addNewWorker();
	static void assignWorker();
	static void deleteWorker(IMWorker<int>*);
};

#endif