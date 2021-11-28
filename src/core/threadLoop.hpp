/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-24 15:39:39
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-28 18:30:18
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
         m_isNeedLoop(false),
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
     * @name: start
     * @Descripttion: start a thread for loop,if you want a threadLoop, you must call start first
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
    std::atomic_bool m_isStop;
    std::atomic_bool m_isNeedLoop;
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

            //if isStop flag was set and  m_itemQueue is empty, exit loop
            //so, if isStop flag was set, but m_itemQueue is not empty, the loop will execute continue untile queue is empty
            if (this->m_isStop && m_itemQueue.empty()) {
                break;
            }
            item = m_itemQueue.front();
            m_itemQueue.pop();

            //if m_itemQueue'size less than m_queueSize, notify it.
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

        //because we need control process rate of item
        //so if item queue size greater than m_queueSize, will block it until item queue size less than m_queueSize
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
void ThreadLoop<T>::start()
{
    m_isNeedLoop = true;
    m_thread = std::thread(&ThreadLoop::_threadLoop, this);
}

template<typename T>
void ThreadLoop<T>::stop()
{
    {
        std::unique_lock<std::mutex> lk(this->m_mutex);
        m_isStop = true;
    }
    m_condition.notify_one();

    if (m_isNeedLoop) {
        //wait thread exit, note it is must do, or else maybe will core dump
        m_thread.join();
    }
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
    //if user don't call stop, must call stop in destructor, and wait thread exit, or else maybe will core dump.
    stop();

}

} //namespace vtf