#ifndef IMOBJECTPOOL_H
#define IMOBJECTPOOL_H

#include <list>
#include <string>
#include <IMThreads/IMWorker.h>

class IMObjectPool
{
    static std::list<IMWorker<int>*> workers;

public:
    static void assignWorker(int);

    static std::list<IMWorker<int>*> getWorkers();

    static IMWorker<int>* getLastWorker();
};

#endif