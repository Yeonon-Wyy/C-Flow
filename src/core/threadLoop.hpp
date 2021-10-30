/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-24 15:39:39
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-30 18:55:06
 */
#pragma once

#include <mutex>
#include <memory>
#include <thread>

namespace vtf {

class ThreadLoop {

public:
    ThreadLoop()
        :m_isStop(false)
    {}

    /**
     * @name: threadLoop
     * @Descripttion: threadLoop is pure virtual function, inheritor must provider a implementation
     * @param {*} none
     * @return {*} true mean continue, false mean stop loop
     */    
    virtual bool threadLoop() = 0;

    /**
     * @name: run
     * @Descripttion: run function will start a thread, this thread will execute threadLoop until it return false or be stoped
     * @param {*}
     * @return {*}
     */    
    void run();

    /**
     * @name: stop
     * @Descripttion: set stop flag, in next loop, the thread loop will stop 
     * @param {*}
     * @return {*}
     */    
    void stop() { m_isStop = true; }
private:
    void _threadLoop();
private:
    bool m_isStop;
    std::shared_ptr<std::thread> m_thread;
};

void ThreadLoop::run()
{
    m_thread = std::make_shared<std::thread>(&ThreadLoop::_threadLoop, this);
}

void ThreadLoop::_threadLoop()
{
    while (true) {
        bool result = this->threadLoop();
        if (!result || m_isStop) break;
    }
}

} //namespace vtf