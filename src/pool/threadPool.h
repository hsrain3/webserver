#ifndef THREAD_POOL
#define THREAD_POOL
#include <mutex>
#include <queue>
#include <condition_variable>

class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount = 8): pool(std::make_shared<Pool>()) {
        assert(threadCount > 0);
        //create threadCount threads
        for(size_t i = 0; i < threadCount;i ++) {
            //use lambda to initialize thread
            std::thread([pool_ = pool]{
                std::unique_lock<std::mutex>locker(pool_->mtx);
                while(true) {
                    if(!pool_->tasks.empty()) {
                        //fetch task
                        auto task = std::move(pool_->tasks.front());
                        pool_->tasks.pop();
                        locker.unlock();
                        task();
                        locker.lock();

                    }
                    else if(pool_->isClosed) break;
                    else pool_->cond.wait(locker);
                }
                //unique 结束会释放锁
            }).detach(); //设置线程分离

        }
    }
    ThreadPool() = default;
    ThreadPool(ThreadPool&&) = default;
    ~ThreadPool() {
        if(static_cast<bool>(pool)) {
            // lock when construct unlock when exit
            std::lock_guard<std::mutex>locker();
            pool->isClosed = true;
        }
        pool->cond.notify_all(); //wake all thread to check if the pool is closed then exit
    }

    template<class T>
    void addTask(T&& task) {
        {
            std::lock_guard<std::mutex>locker(pool->mtx);
            pool->tasks.emplace(std::forward<T>(task)); //still rvalue
        }

        pool->cond.notify_one(); //wake one to execute

    }
private:
    struct Pool {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed;
        std::queue<std::function<void()>> tasks; //类似函数指针？
    };
    std::shared_ptr<Pool>pool;
};



#endif