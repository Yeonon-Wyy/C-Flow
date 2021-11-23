/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 15:37:36
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-23 22:42:41
 */
#pragma once
#include <memory>
#include "utils.hpp"
#include "threadLoop.hpp"
#include "notifier.hpp"

namespace vtf {

#define DISPATCHER_DEFAULT_PREFIX "dispacther_"
constexpr int defaultDsiapctherQueueSize = 8;


template<typename Item>
class Dispatcher : public ThreadLoop<std::shared_ptr<Item>> {
public:
    Dispatcher(int queueSize = defaultDsiapctherQueueSize)
        :ThreadLoop<std::shared_ptr<Item>>(queueSize),
         m_id(m_idGenerator.generate()),
         m_name(DISPATCHER_DEFAULT_PREFIX + vtf::util::StringConvetor::digit2String(m_id))
    {
    }

    Dispatcher(std::string&& name, int queueSize = defaultDsiapctherQueueSize)
        :ThreadLoop<std::shared_ptr<Item>>(queueSize),
         m_id(m_idGenerator.generate()),
         m_name(name)
    {
    }
    
    /**
     * @name: dispatch
     * @Descripttion: according to item info to dispatch item. note it is a pure virtual function, derived class must implement it. this function will call by framework
     * @param {shared_ptr<Item>} item
     * @return {*}
     */    
    virtual bool dispatch(std::shared_ptr<Item> item) = 0;
    
    /**
     * @name: 
     * @Descripttion: queue a item to dispatcher, dispatcher will pop a item in the right time . note it is a pure virtual function, derived class must implement it. this function will call by framework
     * @param {shared_ptr<Item>} item
     * @return {*}
     */    
    virtual void queueInDispacther(std::shared_ptr<Item> item) = 0;

    /**
     * @name: stop
     * @Descripttion: stop dispatcher. note it is a pure virtual function, derived class must implement it. this function will call by framework
     * @param {*}
     * @return {*}
     */    
    virtual void stop() = 0;
private:
    static vtf::util::IDGenerator m_idGenerator;
    long m_id;
    std::string m_name;
};

template<typename Item>
vtf::util::IDGenerator Dispatcher<Item>::m_idGenerator;

} //namesapce vtf