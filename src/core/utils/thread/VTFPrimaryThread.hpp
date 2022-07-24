/*
 * @Author: Yeonon
 * @Date: 2022-07-17 15:08:25
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-07-24 16:43:14
 * @FilePath: /src/core/utils/thread/VTFPrimaryThread.hpp
 * @Description: 
 * Copyright 2022 Yeonon, All Rights Reserved. 
 * 2022-07-17 15:08:25
 */
#pragma once

#include <thread>
#include <functional>
#include "../queue/lockfree_queue.hpp"

#define MAX_TASK_CAPCITY 100

namespace vtf {
namespace utils {
namespace thread {

class VTFPrimaryThread
{
public:
    VTFPrimaryThread()
        :m_tasks(),
         m_stop(false)
    {
        m_thread = std::move(std::thread(&VTFPrimaryThread::execute, this));
    }

    ~VTFPrimaryThread();

    void reset();
private:
    void execute();
    void processTask();
    std::function<void()> popTask();
    void pushTask(std::function<void()>&& func);

private:
    vtf::utils::queue::LockFreeQueue<std::function<void(void)>> m_tasks;
    std::thread m_thread;
    bool m_stop;

    friend class ThreadPool;

};

void VTFPrimaryThread::reset()
{
    m_stop = true;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void VTFPrimaryThread::execute()
{
    while (!m_stop) 
    {
        processTask();
    }
    if (m_stop) {
        //process
        while (!m_tasks.empty()) {
            processTask();
        }
    }
}

void VTFPrimaryThread::pushTask(std::function<void()>&& func)
{
    m_tasks.push(std::move(func));
}

void VTFPrimaryThread::processTask()
{
    auto task = popTask();
    if (task) {
        task();
    } else {
        std::this_thread::yield();
    }
}

std::function<void()> VTFPrimaryThread::popTask()
{
    std::function<void()> task;
    m_tasks.pop(task);
    return task;
}

VTFPrimaryThread::~VTFPrimaryThread()
{
    reset();
}

} //namespace thread
} //namespace utils
} //namespace vtf
