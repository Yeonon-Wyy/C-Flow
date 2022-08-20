/*
 * @Author: Yeonon
 * @Date: 2022-07-17 15:08:25
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-08-06 17:20:58
 * @FilePath: /src/core/utils/thread/VTFPrimaryThread.hpp
 * @Description:
 * Copyright 2022 Yeonon, All Rights Reserved.
 * 2022-07-17 15:08:25
 */
#pragma once

#include <functional>
#include <thread>

#include "../queue/lockfree_queue.hpp"

#define MAX_TASK_CAPCITY 100

namespace vtf
{
namespace utils
{
namespace thread
{
class VTFPrimaryThread
{
   public:
    VTFPrimaryThread() : m_tasks(), m_stop(false), m_totalTaskNum(0) { m_thread = std::move(std::thread(&VTFPrimaryThread::execute, this)); }

    ~VTFPrimaryThread();

    /**
     * @description: reset thread state. need wait current process stop
     * @return {*}
     */
    void reset();

   private:
    /**
     * @description: thread runable functiop
     * @return {*}
     */
    void execute();

    /**
     * @description: get a task from task queue, then process it. if task is invalid, thead will yield
     * @return {*}
     */
    void processTask();

    /**
     * @description: pop a task from task queue
     * @return {*}
     */
    std::function<void()> popTask();

    /**
     * @description: push a task to task queue
     * @param {&&} func
     * @return {*}
     */
    void pushTask(std::function<void()>&& func);

    /**
     * @description: get total task num
     * @return {*}
     */
    int32_t totalTaskNum() const { return m_totalTaskNum; }

   private:
    vtf::utils::queue::LockFreeQueue<std::function<void(void)>> m_tasks;
    std::thread                                                 m_thread;
    bool                                                        m_stop;
    int32_t                                                     m_totalTaskNum;
    friend class ThreadPool;
};

void VTFPrimaryThread::reset()
{
    m_stop = true;
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void VTFPrimaryThread::execute()
{
    while (!m_stop)
    {
        processTask();
        m_totalTaskNum--;
    }
    if (m_stop)
    {
        // process
        while (!m_tasks.empty())
        {
            processTask();
            m_totalTaskNum--;
        }
    }
}

void VTFPrimaryThread::pushTask(std::function<void()>&& func)
{
    m_totalTaskNum++;
    m_tasks.push(std::move(func));
}

void VTFPrimaryThread::processTask()
{
    auto task = popTask();
    if (task)
    {
        task();
    }
    else
    {
        // task is invalid, thead yield. avoid cpu busy-loop
        std::this_thread::yield();
    }
}

std::function<void()> VTFPrimaryThread::popTask()
{
    std::function<void()> task;
    m_tasks.pop(task);
    return task;
}

VTFPrimaryThread::~VTFPrimaryThread() { reset(); }

}  // namespace thread
}  // namespace utils
}  // namespace vtf
