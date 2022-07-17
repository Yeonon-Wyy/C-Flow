/*
 * @Author: Yeonon
 * @Date: 2022-07-17 15:08:25
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-07-17 17:29:52
 * @FilePath: /src/core/utils/thread/VTFPrimaryThread.hpp
 * @Description: 
 * Copyright 2022 Yeonon, All Rights Reserved. 
 * 2022-07-17 15:08:25
 */
#pragma once

#include <thread>
#include <functional>
#include "../queue/blocking_queue.hpp"

#define MAX_TASK_CAPCITY 100

namespace vtf {
namespace utils {
namespace thread {
class VTFPrimaryThread
{
public:
    VTFPrimaryThread()
        :m_tasks(MAX_TASK_CAPCITY),
         m_stop(false)
    {
        m_thread = std::move(std::thread(&VTFPrimaryThread::execute, this));
    }

    ~VTFPrimaryThread();

    void reset();

    bool loadCondition() { return m_tasks.isFull(); }
private:
    void execute();
    void processTask();
    std::function<void()> popTask();

private:
    BlockingQueue<std::function<void()>> m_tasks;
    std::thread m_thread;
    bool m_stop;

    friend class ThreadPool;
};

void VTFPrimaryThread::reset()
{
    m_stop = true;
    m_tasks.stop();
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
        while (!m_tasks.isEmpty()) {
            std::cout << "process leave" << std::endl;
            processTask();
        }
    }
}

void VTFPrimaryThread::processTask()
{
    auto task = popTask();
    if (task)
        task();
}

std::function<void()> VTFPrimaryThread::popTask()
{
    return m_tasks.pop();
}

VTFPrimaryThread::~VTFPrimaryThread()
{
    reset();
}

} //namespace thread
} //namespace util
} //namespace vtf
