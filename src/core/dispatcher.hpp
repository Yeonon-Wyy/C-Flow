/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 15:37:36
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-30 18:49:43
 */
#pragma once
#include <memory>
#include "utils.hpp"
#include "threadLoop.hpp"

namespace vtf {

#define DISPATCHER_DEFAULT_PREFIX "dispacther_"

template<typename Item>
class Dispatcher : public ThreadLoop {
public:
    Dispatcher()
        :m_id(vtf::util::IDGenerator::getInstance()->generate()),
         m_name(DISPATCHER_DEFAULT_PREFIX + vtf::util::StringConvetor::digit2String(m_id))
    {
    }

    Dispatcher(std::string&& name)
        :m_id(vtf::util::IDGenerator::getInstance()->generate()),
         m_name(name)
    {
    }

    virtual bool dispatch(std::shared_ptr<Item> item) = 0;
    virtual void queueInDispacther(std::shared_ptr<Item> item) = 0;
    // virtual bool threadLoop() = 0;
private:
    long m_id;
    std::string m_name;
};



} //namesapce vtf