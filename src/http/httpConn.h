#ifndef HTTP_CONN_H
#define HTTP_CONN_H
#include<atomic>
class HTTPConn {
public:
    static std::atomic<int>userCount;  //connection number

};




#endif