#ifndef WEBSERVER_H
#define WEBSERVER_H
#include"../timer/heaptimer.h"
#include"epoller.h"
#include<memory>
#include<unordered_map>
#include "../http/httpConn.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include "../pool/threadPool.h"
class WebServer{
public:
    WebServer();
    ~WebServer();
    void Start();
private:
    void dealListen();
    void dealWrite(HTTPConn* client);
    void dealRead(HTTPConn* client);
    void CloseConn(HTTPConn* client);
    void sendError(int fd, char* info);
    void addClient(int fd, sockaddr_in addr);
    void extendTime(HTTPConn* client);
    int port;
    bool openLinger; //
    int timeoutMS;
    bool isClose;
    int listenFd;
    uint32_t listenEvent;
    uint32_t connEvent;
    std::unique_ptr<HeapTimer> timer;
    std::unique_ptr<Epoller> epoller;
    std::unordered_map<int, HTTPConn> users; //store each connection
    std::unique_ptr<ThreadPool> threadpool;
    static const int MAX_FD = 65536; //max fd num

};



#endif