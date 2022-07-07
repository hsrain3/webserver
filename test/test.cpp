#include "../src/log/log.h"
#include "../src/pool/threadPool.h"
#include<functional>
#include<features.h>

void TestLog() {
    int cnt = 0, level = 0;
    Log::getInstance()->init(level, "./testLog1", "log", 0);
    for(level = 3; level >= 0; level--) {
        Log::getInstance()->setLevel(level);
        for(int j = 0;j < 10000;j ++) {
            for(int i = 0;i < 4;i ++) {
                LOG_BASE(i,"%s, 1111 %d =====", "Test", cnt++);
            }
        }
    }

    cnt = 0;

    Log::getInstance()->init(level, "./testlog2",".log",5000);
    for(level = 0; level < 4; level++) {
        Log::getInstance()->setLevel(level);
        for(int j = 0;j < 10000;j ++) {
            for(int i = 0;i < 4;i ++) {
                LOG_BASE(i,"%s, 22222222 %d =====", "Test", cnt++);
            }
        }
    }
}

void threadLogTask(int i, int cnt) {
    for(int j = 0;j < 10000;j ++) {
         LOG_BASE(i,"PID:[%04d]======= %05d ========= ", gettid(), cnt++);
    }
}

void TestThreadPool() {
    Log::getInstance()->init(0,"testThreadPool", ".log", 5000);
    ThreadPool threadpool(6);
    for(int i = 0;i < 18;i ++)  {
        threadpool.addTask(std::bind(threadLogTask,i%4,i*10000));
    }
}

int main() {
    TestLog();
    TestThreadPool();
}