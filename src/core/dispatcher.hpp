/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-30 15:37:36
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-07-24 16:48:34
 */
#pragma once
#include <memory>

#include "notifier.hpp"
#include "scheduler.hpp"
#include "utils/id_generator.hpp"
#include "utils/str_convertor.hpp"
#include "utils/thread/threadLoop.hpp"

namespace cflow
{
#define DISPATCHER_DEFAULT_PREFIX "dispacther_"

using namespace cflow::utils::thread;

template <typename Item>
class Dispatcher : public ThreadLoop<std::shared_ptr<Item>, Scheduler>
{
public:
    Dispatcher() : ThreadLoop<std::shared_ptr<Item>, Scheduler>(), m_id(m_idGenerator.generate()), m_name(DISPATCHER_DEFAULT_PREFIX + cflow::utils::StringConvetor::digit2String(m_id)) {}

    Dispatcher(std::string&& name) : ThreadLoop<std::shared_ptr<Item>, Scheduler>(), m_id(m_idGenerator.generate()), m_name(name) {}

    /**
     * @name: dispatch
     * @Descripttion: according to item info to dispatch item. note it is a pure virtual function, derived class must implement it. this function will call by framework
     * @param {shared_ptr<Item>} item
     * @return {*}
     */
    virtual bool dispatch(std::shared_ptr<Item> item) = 0;

    /**
     * @name:
     * @Descripttion: queue a item to dispatcher, dispatcher will pop a item in the right time . note it is a pure virtual function, derived class must implement it. this function will call by
     * framework
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
    static cflow::utils::IDGenerator m_idGenerator;
    cflow_id_t                       m_id;
    std::string                    m_name;
};

template <typename Item>
cflow::utils::IDGenerator Dispatcher<Item>::m_idGenerator;

}  // namespace cflow