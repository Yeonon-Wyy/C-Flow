/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 15:37:36
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-20 17:47:05
 */
#pragma once
#include <memory>
#include "utils.hpp"
#include "threadLoop.hpp"
#include "notifier.hpp"

namespace vtf {

#define DISPATCHER_DEFAULT_PREFIX "dispacther_"

template<typename Item>
class Dispatcher : public ThreadLoop {
public:
    Dispatcher()
        :m_id(m_idGenerator.generate()),
         m_name(DISPATCHER_DEFAULT_PREFIX + vtf::util::StringConvetor::digit2String(m_id))
    {
        run();
    }

    Dispatcher(std::string&& name)
        :m_id(m_idGenerator.generate()),
         m_name(name)
    {
    }
    
    virtual bool dispatch(std::shared_ptr<Item> item) = 0;
    virtual void queueInDispacther(std::shared_ptr<Item> item) = 0;
    virtual void stop() = 0;
private:
    static vtf::util::IDGenerator m_idGenerator;
    long m_id;
    std::string m_name;
};

template<typename Item>
vtf::util::IDGenerator Dispatcher<Item>::m_idGenerator;

} //namesapce vtf