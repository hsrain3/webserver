//
// Created by 王澄雨 on 2022/6/27.
//

#include "sqlconnpool.h"
using namespace std;


SQLConnPool::SQLConnPool() {
    useCount = 0;
    freeCount = 0;
}

SQLConnPool* SQLConnPool::getInstance() {
    static SQLConnPool connPool;
    return &connPool;
}

void SQLConnPool::init(const char *host, int port, const char *user, const char *pwd, const char *dbName,int connSize) {
    assert(connSize > 0);
    for(int i = 0;i < connSize;i ++) {
        MYSQL* sql = nullptr;
        sql = mysql_init(sql);
        if(!sql) {
            LOG_ERROR("MYSQL init error!");
            assert(sql);
        }

        sql = mysql_real_connect(sql,host,user,pwd,dbName,port, nullptr,0);
        if(!sql) {
            LOG_ERROR("MySql Connect Error");
        }
        connQue.push(sql);
    }
    MAX_CONN = connSize;
    sem_init(&semId,0,MAX_CONN);

}

//取出连接
MYSQL* SQLConnPool::getConn() {
    MYSQL* sql = nullptr;
    if(connQue.empty()) {
        LOG_WARN("sqlConnPool busy!");
        return nullptr;
    } //?
    sem_wait(&semId);
    {
        lock_guard<mutex> locker(mtx);
        sql = connQue.front();
        connQue.pop();
    }
    return sql;
}

void SQLConnPool::freeConn(MYSQL *sql) {
    assert(sql);
    lock_guard<mutex> locker(mtx);
    connQue.push(sql);
    sem_post(&semId);
}

void SQLConnPool::closePool() {
    lock_guard<mutex> locker(mtx);
    while(!connQue.empty()) {
        auto item = connQue.front();
        connQue.pop();
        mysql_close(item);
    }
    mysql_library_end();
}

int SQLConnPool::getFreeConnCount() {
    lock_guard<mutex> locker(mtx);
    return connQue.size();
}

SQLConnPool::~SQLConnPool() {
    closePool();
}