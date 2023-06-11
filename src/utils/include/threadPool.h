#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "log.h"
#include "CFlowPrimaryThread.h"

namespace cflow::utils::thread {
class ThreadPool
{
public:
    ThreadPool(size_t threadSize);

    /**
     * @name: emplace
     * @Descripttion: emplace a task to thread pool
     * @param {*}
     * @return {*}
     */
    template <typename F, typename... Args>
    auto emplace(F&& f, Args&&... agrs)
        -> std::future<typename std::result_of<F(Args...)>::type>;

    void stop();

    bool isStoped()
    {
        std::unique_lock<std::mutex> lk(this->m_taskMutex);
        return isStop;
    }

    ~ThreadPool();

private:
    // thread list, we need keep fixed number of threads
    std::vector<std::thread> m_workers;

    // task queue
    std::queue<std::function<void()>> m_tasks;

    // for synchronization
    std::mutex              m_taskMutex;
    std::condition_variable m_taskCV;

    // stop flag
    bool isStop;
};

template <typename F, typename... Args>
auto ThreadPool::emplace(F&& f, Args&&... agrs)
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using returnType = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<returnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(agrs)...));

    std::future<returnType> taskFuture = task->get_future();

    {
        std::unique_lock<std::mutex> lk(this->m_taskMutex);
        if (isStop)
        {
            throw std::runtime_error("emplace on stopped ThreadPool");
        }
        this->m_tasks.emplace([task]() { (*task)(); });
    }

    this->m_taskCV.notify_one();
    return taskFuture;
}

} // namespace cflow::utils::thread