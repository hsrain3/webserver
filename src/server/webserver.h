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
#include "../timer/heaptimer.h"
#include "../log/log.h"
class WebServer{
public:
    WebServer(int port, int trigMode, int timeoutMS, bool OptLinger, 
        int sqlPort, const char* sqlUser, const  char* sqlPwd, 
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize);
    ~WebServer();
    void Start();
private:
    bool initSocket();
    void initEventMode(int trigMode);


    void dealListen();
    void dealWrite(HTTPConn* client);
    void dealRead(HTTPConn* client);
    void closeConn(HTTPConn* client);
    void sendError(int fd, const char* info);
    void addClient(int fd, sockaddr_in addr);
    void extendTime(HTTPConn* client);
    void onRead(HTTPConn* client);
    void onWrite(HTTPConn* client);
    void onProcess(HTTPConn* client);
    int setFdNonblock(int fd);
    int port;
    bool openLinger; //
    int timeoutMS;
    bool isClose;
    int listenFd;
    char* srcDir;
    uint32_t listenEvent;
    uint32_t connEvent;
    std::unique_ptr<HeapTimer> timer;
    std::unique_ptr<Epoller> epoller;
    std::unordered_map<int, HTTPConn> users; //store each connection
    std::unique_ptr<ThreadPool> threadpool;
    static const int MAX_FD = 65536; //max fd num

};



#endif