#include "threadPool.h"

namespace cflow::utils::thread {

ThreadPool::ThreadPool(size_t threadSize) : isStop(false)
{
    for (size_t i = 0; i < threadSize; i++)
    {
        m_workers.emplace_back([this]() {
            while (true)
            {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lk(this->m_taskMutex);
                    this->m_taskCV.wait(lk, [this]() {
                        return this->isStop || !m_tasks.empty();
                    });
                    // if isStop flag is ture and task queue is empty, we just
                    // return else we need execute task, even if isStop flag is
                    // true
                    if (this->isStop && m_tasks.empty())
                    {
                        return;
                    }
                    task = std::move(this->m_tasks.front());
                    this->m_tasks.pop();
                }
                // execute task
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lk(this->m_taskMutex);
        if (isStop)
        {
            return;
        }
    }
    stop();
}

void ThreadPool::stop()
{
    {
        std::unique_lock<std::mutex> lk(this->m_taskMutex);
        isStop = true;
    }

    // notify all wait thread, let remain thread can run
    this->m_taskCV.notify_all();

    // wait all thread run complate
    for (auto& worker : this->m_workers)
    {
        worker.join();
    }
}

} // namespace cflow::utils::thread