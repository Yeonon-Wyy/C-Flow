/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-24 15:39:39
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-09-11 20:44:51
 */
#pragma once

#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "../log/log.hpp"
#include "thread_utils.hpp"

namespace cflow::utils::thread {
constexpr int threadLoopDefaultQueueSize = 8;

/**
 * @name: class ThreadLoop
 * @Descripttion: provide some function of thread alway loop utils condition is
 * reach. Note: a thread loop need some scheduler to control flow.and scheduler
 * object need provide some function like:
 *                1. default constructor
 *                2. "schedule" function
 *                3. "empty","size","emplace" function
 *                4. etc.....
 * @param {*}
 * @return {*}
 */
template <typename T, template <typename> typename Scheduler>
class ThreadLoop
{
public:
    ThreadLoop() : m_isStop(false), m_isNeedLoop(false), m_scheduler() {}

    ~ThreadLoop();

    /**
     * @name: threadLoop
     * @Descripttion: threadLoop is pure virtual function, inheritor must
     * provider a implementation
     * @param {*} none
     * @return {*} true mean continue, false mean stop loop
     */
    virtual bool threadLoop(T item) = 0;

    /**
     * @name: start
     * @Descripttion: start a thread for loop,if you want a threadLoop, you must
     * call start first
     * @param {*}
     * @return {*}
     */
    void start();

    /**
     * @name: stop
     * @Descripttion: set stop flag, in next loop, the thread loop will stop
     * @param {*}
     * @return {*}
     */
    void stop();

    /**
     * @name: queue a item to thread loop, will pop the itme in right time.
     * @Descripttion:
     * @param {*}
     * @return {*}
     */
    void queueItem(T item);

private:
    void _threadLoop();

private:
    std::atomic_bool        m_isStop;
    std::atomic_bool        m_isNeedLoop;
    std::thread             m_thread;
    std::mutex              m_mutex;
    std::condition_variable m_condition;
    std::condition_variable m_not_full_cv;
    Scheduler<T>            m_scheduler;
};

template <typename T, template <typename> typename Scheduler>
void ThreadLoop<T, Scheduler>::_threadLoop()
{
    CFLOW_LOGD("ThreadLoop start");
    while (true)
    {
        T item;
        {
            std::unique_lock<std::mutex> lk(m_mutex);
            m_condition.wait(lk, [this]() {
                return this->m_isStop || !m_scheduler.empty();
            });

            // if isStop flag was set and  m_scheduler is empty, exit loop
            // so, if isStop flag was set, but m_scheduler is not empty, the
            // loop will execute continue untile queue is empty
            if (this->m_isStop && m_scheduler.empty())
            {
                break;
            }
            item = m_scheduler.schedule();
        }

        bool result = this->threadLoop(item);
        if (!result)
        {
            return;
        }
    }
    CFLOW_LOGD("ThreadLoop end");
}

template <typename T, template <typename> typename Scheduler>
void ThreadLoop<T, Scheduler>::queueItem(T item)
{
    {
        std::unique_lock<std::mutex> lk(this->m_mutex);
        if (m_isStop)
        {
            return;
        }
        m_scheduler.emplace(item);
    }
    m_condition.notify_one();
}

template <typename T, template <typename> typename Scheduler>
void ThreadLoop<T, Scheduler>::start()
{
    m_isNeedLoop = true;
    m_thread     = std::thread(&ThreadLoop::_threadLoop, this);
    setScheduling(m_thread, SCHED_FIFO, 30);
}

template <typename T, template <typename> typename Scheduler>
void ThreadLoop<T, Scheduler>::stop()
{
    {
        std::unique_lock<std::mutex> lk(this->m_mutex);
        m_isStop = true;
    }
    m_condition.notify_one();

    if (m_isNeedLoop)
    {
        // wait thread exit, note it is must do, or else maybe will core dump
        m_thread.join();
    }
}

template <typename T, template <typename> typename Scheduler>
ThreadLoop<T, Scheduler>::~ThreadLoop()
{
    {
        std::unique_lock<std::mutex> lk(this->m_mutex);
        if (m_isStop)
        {
            return;
        }
    }
    // if user don't call stop, must call stop in destructor, and wait thread
    // exit, or else maybe will core dump.
    stop();
}

} // namespace cflow::utils::thread