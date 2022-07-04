#ifndef HTTP_CONN_H
#define HTTP_CONN_H
#include<atomic>
#include<stdint.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>
#include <errno.h>
#include <string>
#include "httprequest.h"
#include "httpresponse.h"
#include "../buffer/buffer.h"
//this class is for manage one connection


class HTTPConn {
public:
    HTTPConn();
    ~HTTPConn();
    void init(int sockFd, const sockaddr_in& addr);
    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);
    int toWriteBytes();
    void closeConn();
    bool process();  //业务逻辑
    int getFd() const;
    int getPort() const;
    const char* getIP() const ;
    sockaddr_in getAddr() const;

    bool isKeepAlive ()const {
        return request.isKeepAlive();
    }

    static std::atomic<int>userCount;  //connection number
    static const char* srcDir; //静态资源目录
    static bool isET;
private:

    int fd;

    sockaddr_in addr;
    bool isClose;

    int iovCnt;

    iovec iov[2]; //分散内存?
    Buffer readBuff;
    Buffer writeBuff;
    HTTPRequest request;
    HTTPResponse response;



};




#endif