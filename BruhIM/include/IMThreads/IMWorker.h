#pragma once

#ifndef IMWORKER_H
#define IMWORKER_H

#include <boost/thread.hpp>

using namespace boost;
using namespace boost::this_thread;

template <typename T>
class IMWorker {
	// Worker identifier value.
	T value;

	// Internal pointer to thread instance.
	boost::thread* threadInstance;

	// Marker for worker garbage collection.
	bool markedForGC;

public:
	// Templated constructors.
	IMWorker<T>();
	IMWorker<T>(T);

	// Convenience method to fetch worker identifier value.
	T getIdent();

	// Initialize thread with bound method and context.
	template <typename AT>
	int initializeThread(int (AT::* method)(), AT* ref) {
		threadInstance = new boost::thread(boost::bind(method, ref));
		return 0;
	}

	// Promote thread to higher UAC level.
	int promote();

	// Convenience setter for garbage collection.
	void markForDelete();
};

template <typename T>
inline IMWorker<T>::IMWorker() : value(rand()) { }

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
	// TODO: Raise UAC level on server threads.
	return 0;
}

template<typename T>
inline void IMWorker<T>::markForDelete() {
	markedForGC = true;
}

#endif