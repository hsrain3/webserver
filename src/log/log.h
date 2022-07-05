//
// Created by 王澄雨 on 2022/6/18.
//

#ifndef WEBSERVER_LOG_H
#define WEBSERVER_LOG_H

#include "../buffer/buffer.h"
#include "blockqueue.h"
#include <mutex>
#include <memory>
#include <thread>
#include <cstdio>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <assert.h>
#include<string.h>
#include <errno.h>


class Log {
public:
    void init(int level, const char* path = "./log", const char* suffix  =".log", int maxQueueCapacity = 1024);
    //singleton
    static Log* getInstance();
    static void flushLogThread();
    void write(int level, const char* format,...);
    void flush();
    int getLevel();
    void setLevel(int level);
    bool getIsOpen() { return isOpen;}

private:
    Log();
    void appendLogLevelTitle(int level);
    virtual ~Log();
    void asyncWrite();

   // static Log instance;
    static  const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* path;
    const char* suffix;
    int maxLine; //deprecated
    int lineCount;
    int toDay;


    bool isOpen;

    Buffer buff;
    int level_;
    bool isAsync;
    FILE* fp;
    std::unique_ptr<BlockDeque<std::string>> deque;
    std::unique_ptr<std::thread> writeThread;
    std::mutex mtx;

};


#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::getInstance();\
        if (log->getIsOpen() && log->getLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif //WEBSERVER_LOG_H
