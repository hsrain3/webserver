#ifndef EPOLLER_H
#define EPOLLER_H
#include <unistd.h> 
#include<stdint.h>
#include<sys/epoll.h>
#include<vector>
#include<assert.h>
//封装epoll
class Epoller {
public:
    explicit Epoller(int maxEvent = 1024);
    ~Epoller();
    bool addFd(int fd, uint32_t events);

    bool modFd(int fd, uint32_t events);
    bool delFd(int fd);
    int  wait(int timeoutMs = -1);
    int getEventFd(size_t i) const;
    size_t getEvents(size_t i) const;
private:
    int epollFd; //epoll_create 创建的epoll对象
    std::vector<struct epoll_event>events;



};
#endif