//
// Created by 王澄雨 on 2022/6/29.
//

#ifndef WEBSERVER_SQLCONNRAII_H
#define WEBSERVER_SQLCONNRAII_H

#include "sqlconnpool.h"
/* 资源在对象构造初始化 资源在对象析构时释放*/

class SQLConnRAII{
public:
    SQLConnRAII(MYSQL** sql, SQLConnPool* connPool) {
        assert(connPool);
        *sql = connPool->GetConn();
        sql_ = *sql;
        connPool_ = connPool;
    }
    ~SQLConnRAII(){
        if(sql_) {connPool_->freeConn(sql_)}
    }

private:
    MYSQL *sql_;
    SQLConnPool* connPool_;

};


#endif //WEBSERVER_SQLCONNRAII_H
