#include <iostream>
#include <list>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include "IMObjectPool.h"
#include "../constants.h"

namespace logging = boost::log;

int IMObjectPool::workerCount = 0;

std::list<IMWorker<int>*> IMObjectPool::idleWorkers = std::list<IMWorker<int>*>();
std::list<IMWorker<int>*> IMObjectPool::busyWorkers = std::list<IMWorker<int>*>();

IMWorker<int>* IMObjectPool::addNewWorker() {
	if (idleWorkers.empty()) {
		if (workerCount == constants::MAX_WORKERS) {
			/* Thread sleep till available. */
		}

		IMWorker<int>* newWorker = new IMWorker<int>(rand());
		idleWorkers.push_back(newWorker);
		workerCount++;

		return newWorker;
	}
	else {
		return idleWorkers.front();
	}
}

void IMObjectPool::assignWorker() {

}

void IMObjectPool::deleteWorker(IMWorker<int>* worker) {
	worker->markForDelete();
}
