#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <vector>
#include <memory>
#include <future>

#include "task.hpp"
#include "../log.hpp"

namespace vtf {

class TaskThreadPool {
public:
    //Task Compare, with task priority
    struct TaskComp {
        bool operator() (std::shared_ptr<Task> lhs, std::shared_ptr<Task> rhs)
        {
            return lhs->getPriority() > rhs->getPriority();
        }
    };

    TaskThreadPool(size_t threadSize);

    /**
     * @name: emplaceTask
     * @Descripttion: add a task to thread pool
     * @param {shared_ptr<Task>} task
     * @return {*}
     */    
    auto emplaceTask(std::shared_ptr<Task> task) 
        -> std::future<void>;

    ~TaskThreadPool();
private:
    //thread list, we need keep fixed number of threads
    std::vector<std::thread> m_workers;

    //task queue
    std::priority_queue<
        std::shared_ptr<Task>, 
        std::vector<std::shared_ptr<Task>>, 
        TaskComp> m_tasks;

    //for synchronization 
    std::mutex m_taskMutex;
    std::condition_variable m_taskCV;

    //stop flag
    bool isStop;
    
};

TaskThreadPool::TaskThreadPool(size_t threadSize)
    :isStop(false) 
{
    for (size_t i = 0; i < threadSize; i++) {
        m_workers.emplace_back([this](){
            while (true) {
                
                std::shared_ptr<Task> task;
                
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
                    task = std::move(this->m_tasks.top());
                    this->m_tasks.pop();
                }
                //execute task
                (*task)();
            }
        });
    }
}

auto TaskThreadPool::emplaceTask(std::shared_ptr<Task> task) 
        -> std::future<void>
{
    auto pt = std::make_shared< std::packaged_task<void()> >(
        task->getTaskFunc()
    );

    //set runable, threadPool will execute this function
    task->setRunable([pt](){
        (*pt)();
    });

    std::future<void> taskFuture = pt->get_future();
    {
        std::unique_lock<std::mutex> lk(this->m_taskMutex);
        if (isStop) {
            throw std::runtime_error("emplace on stopped ThreadPool");;
        }
        this->m_tasks.emplace(task);
    }

    this->m_taskCV.notify_one();
    return taskFuture;
}

TaskThreadPool::~TaskThreadPool()
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