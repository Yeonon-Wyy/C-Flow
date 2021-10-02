/*
 * @Descripttion: include Task class
 * @version: V1.0
 * @Author: yeonon
 * @Date: 2021-09-22 21:36:41
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-02 21:27:00
 */
#pragma once

#include <functional>
#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include <initializer_list>
#include <future>

#include "Dag.hpp"
#include "utils.hpp"


#define TASK_NAME_PREFIX "task_"

namespace vtf {


enum TaskPriority {
    NOURGENCY,
    NORMAL,
    URGENCY,
    EXTREME_URGENCY,
};

class Task : public DAGNode {
public:

    /**
     * @name: Task
     * @Descripttion: Task templeate constructor
     * @param {*} f is function object, args is agr list
     * @return {*}
     */
    Task(const std::string& name) 
        :DAGNode(util::IDGenerator::getInstance()->generate()),
         m_name(name) {}

    Task(const std::string& name, int priority) 
        :DAGNode(util::IDGenerator::getInstance()->generate(), priority),
         m_name(name) {}

    //utils function

    /**
     * @name: setName
     * @Descripttion: set task name
     * @param {*} name
     * @return {*}
     */    
    void setName(const std::string& name) { m_name = std::move(name); }

    /**
     * @name: getName
     * @Descripttion: get task name
     * @param {*}
     * @return {*} task name
     */    
    std::string getName() { return m_name; }

    /**
     * @name: getID
     * @Descripttion: get task id
     * @param {*} 
     * @return {*} task id
     */    
    long getID() { return m_ID; }

    template<typename Function, typename... Args>
    auto commit(Function&& f, Args&&... args)
        -> std::shared_ptr<std::packaged_task<typename std::result_of<Function(Args...)>::type()> >;

private:
    std::string m_name;                                  //task name
    long m_ID;                                           //task id
};


template<typename Function, typename... Args>
auto Task::commit(Function&& f, Args&&... args)
    -> std::shared_ptr<std::packaged_task<typename std::result_of<Function(Args...)>::type()> >
{
    using returnType = typename std::result_of<Function(Args...)>::type;
    auto task = std::make_shared< std::packaged_task<returnType()> >(
        std::bind(std::forward<Function>(f), std::forward<Args>(args)...)
    );
    return task;
}

} //namespace vtf