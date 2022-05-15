/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-14 22:58:29
 * @LastEditors: yeonon
 * @LastEditTime: 2022-01-30 21:48:40
 */
#pragma once

#include "utils/thread/threadLoop.hpp"
#include "utils/id_generator.hpp"
#include "utils/str_convertor.hpp"
#include "type.hpp"
#include "scheduler.hpp"

#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <map>

namespace vtf {

#define NOTIFIER_DEFAULT_PREFIX "notfier_default";
constexpr vtf_id_t initExpectItemId = 1;

/**
 * @name: Notifier
 * @Descripttion: Notifier is a common class. user can inheriting this class for user requirement.
 * Notifier don't lock any function, so if user need lock, user should lock self.
 * Note: if notifier type is NotifierType::FINAL, it will start a thread for loop.
 *       And if notier type is DATA_LISTEN or is other "data-feedback-like" will not start a thread, because it's no need for thread, so you shouln only put some light logic in callback.
 */
template<typename Item>
class Notifier : public ThreadLoop<std::shared_ptr<Item> , Scheduler> {
public:
    using NotifierProcessCallback = std::function<bool(std::shared_ptr<Item>)>;
    using ConfigProgress = std::function<void()>;
    using StopProgress = std::function<void()>;
    using NotifyDoneCallback = std::function<void(vtf_id_t)>;

    struct NotifierCreateInfo {
        vtf_id_t id = -1;
        std::string name;
        NotifierProcessCallback processCallback;
        ConfigProgress configProgress;
        StopProgress stopProgress;
        NotifierType type;
        int readyQueueSize;
    };

    class NotifierBuilder {
    public:
        NotifierBuilder()
            :m_id(-1),
             m_type(NotifierType::FINAL)
        {}
        NotifierBuilder* setID(vtf_id_t id);
        NotifierBuilder* setName(const std::string& name);
        NotifierBuilder* setProcessCallback(NotifierProcessCallback&& cb);
        NotifierBuilder* setNotifyDoneCallback(NotifyDoneCallback&& cb);
        NotifierBuilder* setConfigProgress(ConfigProgress&& cp);
        NotifierBuilder* setStopProgress(StopProgress&& sp);
        NotifierBuilder* setType(NotifierType&& type);
        
        std::shared_ptr<Notifier<Item>> build();
    private:
        vtf_id_t m_id;
        std::string m_name;
        NotifierProcessCallback m_processCallback;
        NotifyDoneCallback m_notifyDoneCallback;
        ConfigProgress m_configProgress;
        StopProgress m_stopProgress;
        NotifierType m_type;
    };
public:
    Notifier(vtf_id_t id)
        :ThreadLoop<std::shared_ptr<Item>, Scheduler>(),
         m_id(id),
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
     * @name: ID
     * @Descripttion: return notifier's ID
     * @param {*}
     * @return {*}
     */    
    vtf_id_t ID() const { return m_id; }

    /**
     * @name: config
     * @Descripttion: user-define config progress 
     * @param {*}
     * @return {*}
     */    
    void config();

    /**
     * @name: stop
     * @Descripttion: stop notifier. 
     * @param {*}
     * @return {*}
     */    
    void stop();
private:
    vtf_id_t m_id;
    std::string m_name;
    NotifierProcessCallback m_processCallback;
    NotifyDoneCallback m_notifyDoneCallback;
    ConfigProgress m_configProgress;
    StopProgress m_stopProgress;
    std::map<vtf_id_t, std::shared_ptr<Item>> m_pendingItemMap;
    vtf_id_t m_expectItemId = initExpectItemId;
    NotifierType m_type;
};

/*
*
* Implementation of class Notifier
*
*/

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
        m_notifyDoneCallback(item->ID());
        //update m_expectItemId
        m_expectItemId = item->ID() + 1;
        VTF_LOGD("[{0}] result notifier process item {1}, now expect item id {2}", name(), item->ID(), m_expectItemId);
        
        //if pending map is not empty, mean future item arrive first, we need process it.
        for (auto it = m_pendingItemMap.begin(); it != m_pendingItemMap.end();) {
            //need one-by-one process, so we need juge m_expectItemId and item id.
            if (it->first == m_expectItemId) {
                m_processCallback(it->second);
                m_notifyDoneCallback(it->first);
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
        ThreadLoop<std::shared_ptr<Item>, Scheduler>::queueItem(item);
    }
}

template<typename Item>
void Notifier<Item>::config()
{
    VTF_LOGD("notifier [{0}] config start", m_name);
    if (m_configProgress) {
        m_configProgress();
    }
    VTF_LOGD("notifier [{0}] config end", m_name);
}


template<typename Item>
void Notifier<Item>::stop()
{
    VTF_LOGD("notifier [{0}] stop start", m_name);
    if (m_stopProgress) {
        m_stopProgress();
    }
    ThreadLoop<std::shared_ptr<Item>, Scheduler>::stop();
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
typename Notifier<Item>::NotifierBuilder* Notifier<Item>::NotifierBuilder::setID(vtf_id_t id)
{
    this->m_id = id;
    return this;
}

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
typename Notifier<Item>::NotifierBuilder* Notifier<Item>::NotifierBuilder::setNotifyDoneCallback(NotifyDoneCallback&& cb)
{
    if (m_type == NotifierType::FINAL) {
        m_notifyDoneCallback = std::move(cb);
    }
    return this;
}

template<typename Item>
typename Notifier<Item>::NotifierBuilder* Notifier<Item>::NotifierBuilder::setConfigProgress(ConfigProgress&& cp)
{
    m_configProgress = std::move(cp);
    return this;
}

template<typename Item>
typename Notifier<Item>::NotifierBuilder* Notifier<Item>::NotifierBuilder::setStopProgress(StopProgress&& sp)
{
    m_stopProgress = std::move(sp);
    return this;
}

template<typename Item>
typename Notifier<Item>::NotifierBuilder* Notifier<Item>::NotifierBuilder::setType(NotifierType&& type)
{
    m_type = std::move(type);
    return this;
}

template<typename Item>
std::shared_ptr<Notifier<Item>> Notifier<Item>::NotifierBuilder::build()
{
    if (!m_processCallback) {
        VTF_LOGE("notifier process callback must be given!");
        return nullptr;
    }

    if (m_id == -1) {
        VTF_LOGE("notifier id {0} can't less than 0. please check it.", m_id);
        return nullptr;
    }

    //set field
    std::shared_ptr<Notifier<Item>> notifier = std::make_shared<Notifier<Item>>(m_id);
    if (m_name == "") {
        m_name = NOTIFIER_DEFAULT_PREFIX;
    }

    notifier->m_name = m_name;
    notifier->m_type = m_type;

    if (m_processCallback) {
        notifier->m_processCallback = m_processCallback;
    }
    if (m_configProgress) {
        notifier->m_configProgress = m_configProgress;
    }
    if (m_stopProgress) {
        notifier->m_stopProgress = m_stopProgress;
    }
    if (m_notifyDoneCallback) {
        notifier->m_notifyDoneCallback = m_notifyDoneCallback;
    }

    return notifier;
}

} //namespace vtf