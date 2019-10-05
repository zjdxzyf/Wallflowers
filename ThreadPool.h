#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

namespace mslite {
constexpr size_t MaxThreads = 4;

class ThreadPool {
public:
    static ThreadPool *GetThreadPool(const size_t &Threads);
    void Run();
    template<class F, class... Args>
    auto enqueue(F &&f, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();

private:
    const size_t threads;
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop;
    ThreadPool(const size_t &Threads);
};

}
#endif