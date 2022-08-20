#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "../log/log.hpp"
#include "VTFPrimaryThread.hpp"

namespace vtf
{
namespace utils
{
namespace thread
{
class ThreadPool
{
   public:
    using VTFPrimaryThreadPtr = std::unique_ptr<VTFPrimaryThread>;

    ThreadPool(size_t threadSize);

    /**
     * @name: emplace
     * @Descripttion: emplace a task to thread pool
     * @param {*}
     * @return {*}
     */
    template <typename F, typename... Args>
    auto emplace(F&& f, Args&&... agrs) -> std::future<typename std::result_of<F(Args...)>::type>;

    void stop();

    bool isStoped() { return isStop; }

    int dispatch();

    ~ThreadPool();

   private:
    // thread list, we need keep fixed number of threads
    std::vector<VTFPrimaryThreadPtr> m_workers;
    // stop flag
    bool   isStop;
    size_t m_curIdx;
};

ThreadPool::ThreadPool(size_t threadSize) : isStop(false), m_curIdx(0)
{
    for (size_t i = 0; i < threadSize; i++)
    {
        m_workers.emplace_back(new VTFPrimaryThread());
    }
}

template <typename F, typename... Args>
auto ThreadPool::emplace(F&& f, Args&&... agrs) -> std::future<typename std::result_of<F(Args...)>::type>
{
    using returnType = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<returnType()> >(std::bind(std::forward<F>(f), std::forward<Args>(agrs)...));

    std::future<returnType> taskFuture = task->get_future();

    // round robin selection
    int idx = dispatch();

    m_workers[idx]->pushTask([task]() { (*task)(); });

    return taskFuture;
}

ThreadPool::~ThreadPool()
{
    if (isStop)
    {
        return;
    }

    stop();
}

void ThreadPool::stop()
{
    isStop = true;
    // wait all thread run complate
    m_workers.clear();
}

int ThreadPool::dispatch()
{
    auto selectThread = [this]() {
        int selectIdx = m_curIdx++;
        if (m_curIdx >= m_workers.size())
        {
            m_curIdx = 0;
        }
        int curThreadLoad = m_workers[selectIdx]->totalTaskNum();
        for (int i = 0; i < m_workers.size(); i++)
        {
            if (i == selectIdx) continue;
            int otherThreadLoad = m_workers[i]->totalTaskNum();
            if (curThreadLoad - otherThreadLoad > 4)
            {
                return -1;
            }
        }
        return selectIdx;
    };

    int selectThreadIdx = -1;
    do
    {
        selectThreadIdx = selectThread();
    } while (selectThreadIdx < 0);

    return selectThreadIdx;
}

}  // namespace thread
}  // namespace utils
}  // namespace vtf