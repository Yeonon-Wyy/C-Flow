/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-24 15:39:39
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-21 20:18:05
 */
#pragma once

#include <mutex>
#include <memory>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <queue>

namespace vtf {

constexpr int threadLoopDefaultQueueSize = 8;
template<typename T>
class ThreadLoop {

public:

    ThreadLoop(int queueSize = threadLoopDefaultQueueSize)
        :m_isStop(false),
         m_thread(&ThreadLoop::_threadLoop, this),
         m_queueSize(queueSize)
    {
    }

    ~ThreadLoop();


    /**
     * @name: threadLoop
     * @Descripttion: threadLoop is pure virtual function, inheritor must provider a implementation
     * @param {*} none
     * @return {*} true mean continue, false mean stop loop
     */    
    virtual bool threadLoop(T item) = 0;

    /**
     * @name: stop
     * @Descripttion: set stop flag, in next loop, the thread loop will stop 
     * @param {*}
     * @return {*}
     */    
    void stop();

    void queueItem(T item);
private:
    void _threadLoop();
private:
    std::atomic_bool m_isStop;
    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::condition_variable m_not_full_cv;
    std::queue<T> m_itemQueue;
    int m_queueSize;
};


template<typename T>
void ThreadLoop<T>::_threadLoop()
{
    VTF_LOGD("ThreadLoop start");
    while (true) {
        T item;
        {
            std::unique_lock<std::mutex> lk(m_mutex);
            m_condition.wait(lk, [this]() {
                return this->m_isStop || !m_itemQueue.empty();
            });
            VTF_LOGD("m_itemQUeue size ({0}), stop flag {1}", m_itemQueue.size(), m_isStop);
            if (this->m_isStop && m_itemQueue.empty()) {
                return;
            }
            item = m_itemQueue.front();
            m_itemQueue.pop();
            if (m_itemQueue.size() < m_queueSize) {
                m_not_full_cv.notify_one();
            }
        }

        bool result = this->threadLoop(item);
        if (!result) {
            return;
        }
    }
    VTF_LOGD("ThreadLoop end");
}

template<typename T>
void ThreadLoop<T>::queueItem(T item)
{
    {
        std::unique_lock<std::mutex> lk(this->m_mutex);
        if (m_isStop) {
            return;
        }
        if (this->m_itemQueue.size() >= m_queueSize) {
            m_not_full_cv.wait(lk, [this](){
                return this->m_itemQueue.size() < m_queueSize;
            });
        }
        this->m_itemQueue.push(item);
    }
    m_condition.notify_one();
}

template<typename T>
void ThreadLoop<T>::stop()
{
    {
        std::unique_lock<std::mutex> lk(this->m_mutex);
        m_isStop = true;
    }
    m_condition.notify_one();
    m_thread.join();
}

template<typename T>
ThreadLoop<T>::~ThreadLoop()
{
    {
        std::unique_lock<std::mutex> lk(this->m_mutex);
        if (m_isStop) {
            return;
        }
    }

    stop();

}

} //namespace vtf