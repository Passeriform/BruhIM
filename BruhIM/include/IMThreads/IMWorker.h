#pragma once

#ifndef IMWORKER_H
#define IMWORKER_H

#include <boost/thread.hpp>

using namespace boost;
using namespace boost::this_thread;

template <typename T>
class IMWorker {
	T value;
	boost::thread* threadInstance;
	bool markedForGC;
public:
	IMWorker<T>();
	IMWorker<T>(T);
	T getIdent();
	template <typename AT>
	int initializeThread(int (AT::* method)(), AT* ref) {
		threadInstance = new boost::thread(boost::bind(method, ref));
		return 0;
	}
	int promote();
	void markForDelete();
};

template <typename T>
inline IMWorker<T>::IMWorker() :
	value(rand()) { }

template <typename T>
inline IMWorker<T>::IMWorker(T ident) : IMWorker() {
	value = ident;
}

template <typename T>
inline T IMWorker<T>::getIdent() {
	return value;
}

template<typename T>
inline int IMWorker<T>::promote() {
	return 0;
}

template<typename T>
inline void IMWorker<T>::markForDelete() {
	markedForGC = true;
}

#endif