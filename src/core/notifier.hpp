/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-14 22:58:29
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-28 18:56:41
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

enum NotifierType {
    NOTIFIER_TYPE_START,
    DATA_LISTEN,
    FINAL,
    NOTIFIER_TYPE_END
};

enum NotifyStatus {
    OK,
    ERROR
};

#define NOTIFIER_DEFAULT_PREFIX "notfier_default";
constexpr long initExpectItemId = 1;
constexpr int defaultNotifierQueueSize = 8;

/**
 * @name: Notifier
 * @Descripttion: Notifier is a common class. user can inheriting this class for user requirement.
 * Notifier don't lock any function, so if user need lock, user should lock self.
 * Note: if notifier type is NotifierType::FINAL, it will start a thread for loop.
 *       And if notier type is DATA_LISTEN or is other "data-feedback-like" will not start a thread, because it's no need for thread, so you shouln only put some light logic in callback.
 */
template<typename Item>
class Notifier : public ThreadLoop<std::shared_ptr<Item>> {
public:
    using NotifierProcessCallback = std::function<bool(std::shared_ptr<Item>)>;

    struct NotifierCreateInfo {
        std::string name;
        NotifierProcessCallback processCallback;
        NotifierType type;
        int readyQueueSize;
    };

    class NotifierBuilder {
    public:
        NotifierBuilder()
            :m_type(NotifierType::FINAL),
             m_readyQueueSize(defaultNotifierQueueSize)
        {}
        NotifierBuilder* setName(const std::string& name);
        NotifierBuilder* setProcessCallback(NotifierProcessCallback&& cb);
        NotifierBuilder* setType(NotifierType&& type);
        NotifierBuilder* setQueueSize(int readyQueueSize);
        
        std::shared_ptr<Notifier<Item>> build();
    private:
        std::string m_name;
        NotifierProcessCallback m_processCallback;
        NotifierType m_type;
        int m_readyQueueSize;
    };
public:

    Notifier(int readyQueueSize = defaultNotifierQueueSize)
        :ThreadLoop<std::shared_ptr<Item>>(readyQueueSize),
         m_id(m_idGenerator.generate()),
         m_expectItemId(initExpectItemId)
    {
    }
    
    ~Notifier()
    {
        VTF_LOGD("notifier {0} destory", m_name);
    }

    static NotifierBuilder builder() { return NotifierBuilder(); }

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
     * @name: type
     * @Descripttion: return notifier's type
     * @param {*}
     * @return {*}
     */    
    NotifierType type() { return m_type; }

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
    NotifierProcessCallback m_processCallback;
    std::map<long, std::shared_ptr<Item>> m_pendingItemMap;
    long m_expectItemId = initExpectItemId;
    NotifierType m_type;
};

/*
*
* Implementation of class Notifier
*
*/
template<typename Item>
vtf::util::IDGenerator Notifier<Item>::m_idGenerator;

template<typename Item>
bool Notifier<Item>::threadLoop(std::shared_ptr<Item> item)
{
    bool ret = true;
    if (!m_processCallback) {
        VTF_LOGD("user must give a process function for notify.");
        return false;
    }
    if (item->ID() == m_expectItemId) {
        //if current item id equal m_expectItemId

        //process current item first
        ret = m_processCallback(item);
        //update m_expectItemId
        m_expectItemId = item->ID() + 1;
        VTF_LOGD("[{0}] result notifier process item {1}, now expect item id {2}", name(), item->ID(), m_expectItemId);
        
        //if pending map is not empty, mean future item arrive first, we need process it.
        for (auto it = m_pendingItemMap.begin(); it != m_pendingItemMap.end();) {
            //need one-by-one process, so we need juge m_expectItemId and item id.
            if (it->first == m_expectItemId) {
                m_processCallback(it->second);
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
    if (m_type == NotifierType::DATA_LISTEN) {
        //just call directly, please ensure the process callback don't need a lot of time, or else will effect performance
        m_processCallback(item);
    } else {
        ThreadLoop<std::shared_ptr<Item>>::queueItem(item);
    }
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

/*
*
* Implementation of class Notifier::NotifierBuilder
*
*/
template<typename Item>
typename Notifier<Item>::NotifierBuilder* Notifier<Item>::NotifierBuilder::setName(const std::string& name)
{
    m_name = name;
    return this;
}
template<typename Item>
typename Notifier<Item>::NotifierBuilder* Notifier<Item>::NotifierBuilder::setProcessCallback(NotifierProcessCallback&& cb)
{
    m_processCallback = std::move(cb);
    return this;
}

template<typename Item>
typename Notifier<Item>::NotifierBuilder* Notifier<Item>::NotifierBuilder::setType(NotifierType&& type)
{
    m_type = std::move(type);
    return this;
}

template<typename Item>
typename Notifier<Item>::NotifierBuilder* Notifier<Item>::NotifierBuilder::setQueueSize(int readyQueueSize)
{
    m_readyQueueSize = readyQueueSize;
    return this;
}

template<typename Item>
std::shared_ptr<Notifier<Item>> Notifier<Item>::NotifierBuilder::build()
{
    if (m_type == NotifierType::FINAL && m_readyQueueSize <= 0) {
        VTF_LOGE("notifier queue size can't less than 1");
        return nullptr;
    }
    if (!m_processCallback) {
        VTF_LOGE("notifier process callback must be given!");
        return nullptr;
    }

    //set field
    std::shared_ptr<Notifier<Item>> notifier = std::make_shared<Notifier<Item>>(m_readyQueueSize);
    if (m_name == "") {
        m_name = NOTIFIER_DEFAULT_PREFIX;
    }

    notifier->m_name = m_name;
    notifier->m_processCallback = m_processCallback;
    notifier->m_type = m_type;


    return notifier;
}

}