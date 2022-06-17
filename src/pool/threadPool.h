#ifndef THREAD_POOL
#define THREAD_POOL
class ThreadPool {
public:
    template<class T>
    void addTask(T&& task);
};



#endif