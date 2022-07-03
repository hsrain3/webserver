#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H
#include <functional>
#include <time.h>
#include <chrono>
#include <vector>
#include <unordered_map>
typedef std::function<void> TimeoutCallback;
typedef std::chrono::hight_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef std::chrono::time_point TimeStamp;

struct TimerNode{
    int id;
    TimeStamp expires;
    TimeoutCallback cb;
    bool operator<(const TimerNode& t) {
        return expires < t.expires;
    }
};



class HeapTimer{
public:
    HeapTimer() {heap.reserve(64)};
    ~HeapTimer(); {clear();}
    void add(int id, int timeOut, const )
    int GetNextTick();
    void adjust(int id, int newExpires);
    void add(int id, int timeOut, const TimeoutCallback& cb);
    void doWork(int id);
    void clear();
    void tick();
    void pop();

private:
    void del(size_t i);
    void siftup(size_t i);
    bool siftdown(size_t index, size_t n);
    void swapNode(size_t i,size_t j);
    std::vector<TimerNode> heap;
    std::unordered_map<int, size_t> ref;

};



#endif