/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-14 22:58:29
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-21 19:16:11
 */
#pragma once

#include "threadLoop.hpp"
#include "blocking_queue.hpp"
#include "utils.hpp"
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>

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
class Notifier : public ThreadLoop<std::shared_ptr<Item>> {
public:
    using ReadyQueue = BlockingQueue<std::shared_ptr<Item>>;

    using NotifierProcessFunction = std::function<bool(std::shared_ptr<Item>)>;

    Notifier(const std::string& name, int readyQueueSize, NotifierProcessFunction&& pf)
        :m_id(m_idGenerator.generate()),
         m_name(name),
         m_processFunction(std::move(pf))
    {
    }
    
    ~Notifier()
    {
        VTF_LOGD("notifier {0} destory", m_name);
    }

    /**
     * @name: threadLoop
     * @Descripttion: 
     * @param {*}
     * @return {*}
     */    
    bool threadLoop(std::shared_ptr<Item>);

    /**
     * @name: notify
     * @Descripttion: queue a item to  m_readyQueue
     * @param {*}
     * @return {*}
     */    
    void notify(std::shared_ptr<Item>);

    std::string name() { return m_name; }

    void stop();
private:
    static vtf::util::IDGenerator m_idGenerator;
    long m_id;
    std::string m_name;
    NotifierProcessFunction m_processFunction;
};

template<typename Item>
vtf::util::IDGenerator Notifier<Item>::m_idGenerator;

template<typename Item>
bool Notifier<Item>::threadLoop(std::shared_ptr<Item> item)
{
    bool ret = true;
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
    ThreadLoop<std::shared_ptr<Item>>::queueItem(item);
}

template<typename Item>
void Notifier<Item>::stop()
{
    VTF_LOGD("notifier [{0}] stop start", m_name);
    ThreadLoop<std::shared_ptr<Item>>::stop();
    VTF_LOGD("notifier [{0}] stop end", m_name);
}

}