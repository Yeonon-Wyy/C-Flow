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
    using CFlowPrimaryThreadPtr = std::unique_ptr<CFlowPrimaryThread>;

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

    bool isStoped() { return isStop; }

    int dispatch();

    ~ThreadPool();

private:
    // thread list, we need keep fixed number of threads
    std::vector<CFlowPrimaryThreadPtr> m_workers;
    // stop flag
    bool   isStop;
    size_t m_curIdx;
};

template <typename F, typename... Args>
auto ThreadPool::emplace(F&& f, Args&&... agrs)
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using returnType = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<returnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(agrs)...));

    std::future<returnType> taskFuture = task->get_future();

    // round robin selection
    int idx = dispatch();

    m_workers[idx]->pushTask([task]() { (*task)(); });

    return taskFuture;
}

} // namespace cflow::utils::thread