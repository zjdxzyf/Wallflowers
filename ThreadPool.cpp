#include <iostream>
#include "ThreadPool.h"

namespace mslite {
inline ThreadPool::ThreadPool(const size_t &Threads) : threads(Threads), stop(false)
{
}

ThreadPool *ThreadPool::GetThreadPool(const size_t &Threads)
{
    if (Threads > MaxThreads) {
        return nullptr;
    }

    static ThreadPool threadPool(Threads);
    return &threadPool;
}

void ThreadPool::Run()
{
    for (size_t i = 0; i < threads; i++) {
        workers.emplace_back(
                [this] {
                    for (;;) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->mtx);
                            this->cv.wait(lock,
                                    [this] {
                                return this->stop || !this->tasks.empty();
                            });
                            if (this->stop && this->tasks.empty())
                                return;
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }
                        task();
                    }
                });
    }

}

template<class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type>
{
    using RetType = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<RetType>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<RetType> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(this->mtx);
        if (stop == true) {
            std::cout << "Enqueueing a task to a stopped queue!" << std::endl;
            return res;
        } else {
            tasks.emplace([task](){(*task)();});
        }
    }
    cv.notify_one();
    return res;
}

inline ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(this->mtx);
        stop = true;
    }
    cv.notify_all();
    for (auto &worker : workers) {
        worker.join();
    }
}
}