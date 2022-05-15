#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <vector>
#include <memory>
#include <future>

#include "../log/log.hpp"

namespace vtf {

class ThreadPool {
public:
    ThreadPool(size_t threadSize);

    /**
     * @name: emplace
     * @Descripttion: emplace a task to thread pool
     * @param {*}
     * @return {*}
     */    
    template<typename F, typename... Args>
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
    //thread list, we need keep fixed number of threads
    std::vector<std::thread> m_workers;

    //task queue
    std::queue<std::function<void()>> m_tasks;

    //for synchronization 
    std::mutex m_taskMutex;
    std::condition_variable m_taskCV;

    //stop flag
    bool isStop;
    
};

ThreadPool::ThreadPool(size_t threadSize)
    :isStop(false) 
{
    for (size_t i = 0; i < threadSize; i++) {
        m_workers.emplace_back([this](){
            while (true) {
                
                std::function<void()> task;
                
                {
                    std::unique_lock<std::mutex> lk(this->m_taskMutex);
                    this->m_taskCV.wait(lk, [this](){
                        return this->isStop || !m_tasks.empty();
                    });
                    //if isStop flag is ture and task queue is empty, we just return
                    //else we need execute task, even if isStop flag is true
                    if (this->isStop && m_tasks.empty()) {
                        return;
                    }
                    task = std::move(this->m_tasks.front());
                    this->m_tasks.pop();
                }
                //execute task
                task();
            }
        });
    }
}

template<typename F, typename... Args>
auto ThreadPool::emplace(F&& f, Args&&... agrs) 
        -> std::future<typename std::result_of<F(Args...)>::type>
{
    using returnType = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<returnType()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(agrs)...)
    );

    std::future<returnType> taskFuture = task->get_future();

    {
        std::unique_lock<std::mutex> lk(this->m_taskMutex);
        if (isStop) {
            throw std::runtime_error("emplace on stopped ThreadPool");;
        }
        this->m_tasks.emplace([task](){
            (*task)();
        });
    }

    this->m_taskCV.notify_one();
    return taskFuture;
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lk(this->m_taskMutex);
        if (isStop) {
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

    //notify all wait thread, let remain thread can run
    this->m_taskCV.notify_all();

    //wait all thread run complate
    for (auto& worker : this->m_workers) {
        worker.join();
    }
}

} //namespace vtf