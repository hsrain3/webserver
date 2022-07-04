//
// Created by 王澄雨 on 2022/6/26.
//
#include "log.h"
using namespace std;

//?
Log* Log::getInstance() {
    static Log instance;
    return &instance;
}

Log::Log() {
    lineCount = 0;
    isAsync = false;
    writeThread = nullptr;
    deque = nullptr;
    toDay = 0;
    fp = nullptr;
}

Log::~Log() {
    if(writeThread && writeThread->joinable()) {
        while(!deque->empty()) {
            deque->flush();
        }
        deque->close();
        writeThread->join();

    }
    if(fp) {
        lock_guard<mutex> locker(mtx);
        flush();
        fclose(fp);
    }

}
int Log::getLevel() {
    lock_guard<mutex> locker(mtx);
    return level;
}

void Log::setLevel(int level) {
    lock_guard<mutex> locker(mtx);
    level = level;
}

void Log::init(int level = 1, const char *path, const char *suffix, int maxQueueCapacity) {
    isOpen = true;
    this->level = level;
    if(maxQueueCapacity > 0) {
        isAsync = true;
        if(!deque) {
            unique_ptr<BlockDeque<string>> newDeque(new BlockDeque<string>);
            deque = move(newDeque);
            std::unique_ptr<thread> newThread(new thread(flushLogThread));
            writeThread = move(newThread);
        }
    } else {
        isAsync = false;
    }
    lineCount = 0;
    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;

    this->path = path;
    this->suffix = suffix;

    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s",
             this->path, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, this->suffix);
    toDay = t.tm_mday;

    {
        lock_guard<mutex> locker(mtx);
        buff.retrieveAll();
        if(fp) {
            flush(); //?
            fclose(fp);
        }

        fp = fopen(fileName, "a");
        if(fp == nullptr) {
            mkdir(path, 0777);
            fp = fopen(fileName, "a");
        }
        assert(fp != nullptr);
    }

}

void Log::write(int level, const char *format, ...) {
    timeval now = {0,0};
    gettimeofday(&now,nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list valist;
    if(toDay != t.tm_mday || (lineCount&&(lineCount%MAX_LINES == 0))) {
        unique_lock<mutex> locker(mtx);
        locker.unlock();
        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year+1900, t.tm_mon + 1, t.tm_mday);
        if(toDay != t.tm_mday) {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path, tail, suffix);
            toDay = t.tm_mday;
            lineCount = 0;
        } else {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path, tail, (lineCount/MAX_LINES), suffix);
        }
        locker.lock();
        flush();
        fclose(fp);
        fp = fopen(newFile, "a");
        assert(fp != nullptr);

    }
    {
        unique_lock<mutex> locker(mtx);
        lineCount ++;
        int n = snprintf(buff.beginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                         t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                         t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        buff.hasWritten(n);
        appendLogLevelTitle(level);
        va_start(valist,format);
        int m = vsnprintf(buff.beginWrite(),buff.writableBytes(),format, valist);
        va_end(valist);

        buff.hasWritten(m);
        buff.append("\n\0",2);
        if(isAsync&&deque&&!deque->full()) {
            deque->push_back(buff.retrieveAllToStr());
        } else {
            fputs(buff.peek(),fp);
        }
        buff.retrieveAll();

    }
}

void Log::appendLogLevelTitle(int level) {
    switch(level) {
        case 0:
            buff.append("[debug]: ", 9);
            break;
        case 1:
            buff.append("[info]: ", 9);
            break;
        case 2:
            buff.append("[warn]: ", 9);
            break;
        case 3:
            buff.append("[error]: ", 9);
            break;
        default:
            buff.append("info: ", 9);
            break;
    }
}

void Log::flush() {
    if(isAsync) {
        deque->flush();
    }
    fflush(fp);
}

void Log::asyncWrite() {
    string  str = "";
    while(deque->pop(str)) {
        lock_guard<mutex>locker(mtx);
        fputs(str.c_str(),fp);
    }
}

void Log::flushLogThread() {
    Log::getInstance()->asyncWrite();
}