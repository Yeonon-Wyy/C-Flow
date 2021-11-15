/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-14 22:58:29
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-15 23:28:47
 */
#pragma once

#include "threadLoop.hpp"
#include "blocking_queue.hpp"
#include "utils.hpp"
#include <mutex>
#include <condition_variable>
#include <memory>

namespace vtf {


#define NOTIFIER_DEFAULT_PREFIX "notfier_";

/**
 * @name: Notifier
 * @Descripttion: Notifier is a common class. user can inheriting this class for user requirement.
 * Notifier don't lock any function, so if user need lock, user should lock self.
 * @param {*}
 * @return {*}
 */
template<typename Item>
class Notifier : public ThreadLoop {
public:
    using ReadyQueue = BlockingQueue<std::shared_ptr<Item>>;

    using NotifierProcessFunction = std::function<bool(std::shared_ptr<Item>)>;

    Notifier(const std::string& name, int readyQueueSize, NotifierProcessFunction&& pf)
        :m_id(m_idGenerator.generate()),
         m_name(name),
         m_readyQueue(readyQueueSize),
         m_processFunction(std::move(pf))
    {
        run();
    }

    /**
     * @name: threadLoop
     * @Descripttion: 
     * @param {*}
     * @return {*}
     */    
    bool threadLoop();

    /**
     * @name: notify
     * @Descripttion: queue a item to  m_readyQueue
     * @param {*}
     * @return {*}
     */    
    void notify(std::shared_ptr<Item>);

    std::string name() { return m_name; }
private:
    static vtf::util::IDGenerator m_idGenerator;
    long m_id;
    std::string m_name;
    ReadyQueue m_readyQueue;
    NotifierProcessFunction m_processFunction;
};

template<typename Item>
vtf::util::IDGenerator Notifier<Item>::m_idGenerator;

template<typename Item>
bool Notifier<Item>::threadLoop()
{
    bool ret = true;
    auto item = m_readyQueue.pop();
    if (!m_processFunction) {
        VTF_LOGD("user must give a process function for notify.");
        return false;
    }
    ret = m_processFunction(item);
    VTF_LOGD("[{0}] result notifier process item {1}", name(), item->ID());
    return ret;
}

template<typename Item>
void Notifier<Item>::notify(std::shared_ptr<Item> item)
{
    m_readyQueue.push(item);
}

}