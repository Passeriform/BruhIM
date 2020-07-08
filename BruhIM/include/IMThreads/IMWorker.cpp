#include <string>
#include <IMThreads/IMWorker.h>

template <typename T>
IMWorker<T>::IMWorker(T ident) {
	value = ident;
}

template <typename T>
T IMWorker<T>::getIdent() {
	return value;
}

template <typename T>
void IMWorker<T>::setIdent(T ident) {
	value = ident;
}

template class IMWorker<int>;

