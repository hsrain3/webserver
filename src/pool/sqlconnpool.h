//
// Created by 王澄雨 on 2022/6/27.
//

#ifndef WEBSERVER_SQLCONNPOOL_H
#define WEBSERVER_SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <semaphore.h>
#include "../log/log.h"
#include <queue>
#include <mutex>
#include <thread>


class SQLConnPool {
public:
    static SQLConnPool* getInstance();
    MYSQL* getConn();
    void freeConn(MYSQL* conn);
    int getFreeConnCount();

    void init(const char* host, int port, const char* user, const char* pwd, const char* dbName, int connSize = 0);
    void closePool();

private:
    SQLConnPool();
    ~SQLConnPool();

    int MAX_CONN; //最大连接数
    int useCount; //当前用户数
    int freeCount; //空闲用户数

    std::queue<MYSQL*> connQue; //存放空闲连接
    std::mutex mtx;
    sem_t semId;
};

#endif //WEBSERVER_SQLCONNPOOL_H
