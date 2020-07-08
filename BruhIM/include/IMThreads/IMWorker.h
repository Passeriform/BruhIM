#ifndef IMWORKER_H
#define IMWORKER_H

template <typename T>
class IMWorker
{
    T value;
public:
    IMWorker<T>(T);
    T getIdent();
    void setIdent(T);
};

#endif