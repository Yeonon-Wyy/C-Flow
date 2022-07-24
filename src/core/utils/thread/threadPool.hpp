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
#include "VTFPrimaryThread.hpp"

namespace vtf {
namespace utils {
namespace thread {

class ThreadPool {
public:
    using VTFPrimaryThreadPtr = std::unique_ptr<VTFPrimaryThread>;

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
        return isStop;
    }

    ~ThreadPool();
private:
    //thread list, we need keep fixed number of threads
    std::vector<VTFPrimaryThreadPtr> m_workers;
    //task queue
    // std::queue<std::function<void()>> m_tasks;
    //stop flag
    bool isStop;
    size_t m_curIdx;
    
};

ThreadPool::ThreadPool(size_t threadSize)
    :isStop(false),
     m_curIdx(0)
{
    for (size_t i = 0; i < threadSize; i++) {
        m_workers.emplace_back(new VTFPrimaryThread());
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

    //round robin selection
    auto dispatch = [this](){
        int selectIdx = m_curIdx++;
        if (m_curIdx >= m_workers.size()) {
            m_curIdx = 0;
        }
        return selectIdx;
    };
    int idx = dispatch();
    
    m_workers[idx]->pushTask([task](){
        (*task)();
    });
    
    return taskFuture;
}


ThreadPool::~ThreadPool()
{
    
    if (isStop) {
        return;
    }
    
    stop();
}

void ThreadPool::stop()
{
    
    isStop = true;
    //wait all thread run complate
    m_workers.clear();
}

} //namespace thread
} //namespace utils
} //namespace vtf