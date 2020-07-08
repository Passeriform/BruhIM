#include <iostream>
#include <list>
#include <IMThreads/IMObjectPool.h>

std::list<IMWorker<int>*> IMObjectPool::workers = std::list<IMWorker<int>*>({ new IMWorker<int>(0) });

void IMObjectPool::assignWorker(int value) {
    IMObjectPool::workers.push_back(new IMWorker<int>(value));
}

std::list<IMWorker<int>*> IMObjectPool::getWorkers() {
    return IMObjectPool::workers;
}

IMWorker<int>* IMObjectPool::getLastWorker() {
    IMWorker<int>* worker = IMObjectPool::workers.front();
    workers.pop_front();
    return worker;
}