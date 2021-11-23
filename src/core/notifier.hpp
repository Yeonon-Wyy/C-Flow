/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-14 22:58:29
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-23 22:39:01
 */
#pragma once

#include "threadLoop.hpp"
#include "utils.hpp"


#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <map>

namespace vtf {

using NotifierType = uint32_t;

enum NotifyStatus {
    OK,
    ERROR
};

#define NOTIFIER_DEFAULT_PREFIX "notfier_";
constexpr long initExpectItemId = 1;
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
    using NotifierProcessFunction = std::function<bool(std::shared_ptr<Item>)>;

    Notifier(const std::string& name, int readyQueueSize, NotifierProcessFunction&& pf)
        :ThreadLoop<std::shared_ptr<Item>>(readyQueueSize),
         m_id(m_idGenerator.generate()),
         m_name(name),
         m_processFunction(std::move(pf)),
         m_expectItemId(initExpectItemId)
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

    /**
     * @name: name
     * @Descripttion: return notifier's name
     * @param {*}
     * @return {*}
     */    
    std::string name() { return m_name; }

    /**
     * @name: stop
     * @Descripttion: stop notifier. 
     * @param {*}
     * @return {*}
     */    
    void stop();
private:
    static vtf::util::IDGenerator m_idGenerator;
    long m_id;
    std::string m_name;
    NotifierProcessFunction m_processFunction;
    std::map<long, std::shared_ptr<Item>> m_pendingItemMap;
    long m_expectItemId = initExpectItemId;
    NotifierType m_type;
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
    if (item->ID() == m_expectItemId) {
        //if current item id equal m_expectItemId

        //process current item first
        ret = m_processFunction(item);
        //update m_expectItemId
        m_expectItemId = item->ID() + 1;
        VTF_LOGD("[{0}] result notifier process item {1}, now expect item id {2}", name(), item->ID(), m_expectItemId);
        
        //if pending map is not empty, mean future item arrive first, we need process it.
        for (auto it = m_pendingItemMap.begin(); it != m_pendingItemMap.end();) {
            //need one-by-one process, so we need juge m_expectItemId and item id.
            if (it->first == m_expectItemId) {
                m_processFunction(it->second);
                //update m_expectItemId
                m_expectItemId = it->second->ID() + 1;
                VTF_LOGD("[{0}] result notifier process item {1}, now expect item id {2}", name(), it->second->ID(), m_expectItemId);
                m_pendingItemMap.erase(it++);
            } else {
                it++;
                break;
            }
        }
    } else {
        //if current item not equal m_expectItemId, will hold it to pending map
        m_pendingItemMap[item->ID()] = item;
    }
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
    m_pendingItemMap.clear();
    m_expectItemId = initExpectItemId;
    VTF_LOGD("notifier [{0}] stop end", m_name);
}

}