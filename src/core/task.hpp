/*
 * @Descripttion: include Task class
 * @version: V1.0
 * @Author: yeonon
 * @Date: 2021-09-22 21:36:41
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-10 21:26:29
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
     * @Descripttion: Task constructor
     * @param {*} name is the task name
     * @return {*} 
     */
    Task(const std::string& name) 
        :DAGNode(util::IDGenerator::getInstance()->generate()),
         m_name(name),
         m_ID(getNodeId()) {}

    /**
     * @name: Task 
     * @Descripttion: Task constroctr 
     * @param {int} priority 
     * @return {*}
     */    
    Task(const std::string& name, int priority) 
        :DAGNode(util::IDGenerator::getInstance()->generate()),
         m_name(name),
         m_priority(priority),
         m_ID(getNodeId()) {}

    /**
     * @name: commit
     * @Descripttion: commit a task, but not execute task for now. just construct a package_task object than retrun it.
     * @param {*}
     * @return {*} a package_task object
     */    
    template<typename Function, typename... Args>
    auto commit(Function&& f, Args&&... args)
        -> std::shared_ptr<std::packaged_task<typename std::result_of<Function(Args...)>::type()> >;


    /* setter and getter function  */
    void setName(const std::string& name) { m_name = std::move(name); }
    std::string getName() { return m_name; }

    long getID() { return m_ID; }

    void setPriority(int priority) { m_priority = priority; }
    int getPriority() { return m_priority; }

    /**
     * @name: getTaskFunc
     * @Descripttion: getTaskFunc
     * @param {*}
     * @return {*} m_execFunc
     */    
    std::function<void()> getTaskFunc() { return m_taskFunc; }

    /**
     * @name: setRunFunc
     * @Descripttion: just set run function to task object
     * @param {*}
     * @return {*}
     */    
    void setRunable(std::function<void()>&& runable) { m_runable = std::move(runable); }

    /**
     * @name: will execute runanble function, must be set
     * @Descripttion: 
     * @param {*}
     * @return {*}
     */    
    void operator() () 
    { 
        if (m_runable) {
            m_runable(); 
        } else {
            throw std::runtime_error("before execute, must set runable");;
        }
    }

private:
    std::string m_name;                                  //task name
    long m_ID;                                           //task id
    std::function<void()> m_taskFunc;                    //task function, will execute user task
    int m_priority;                                      //priority
    std::function<void()> m_runable;                     //runable function, only for threadPool

};


template<typename Function, typename... Args>
auto Task::commit(Function&& f, Args&&... args)
    -> std::shared_ptr<std::packaged_task<typename std::result_of<Function(Args...)>::type()> >
{
    using returnType = typename std::result_of<Function(Args...)>::type;

    auto pt = std::make_shared< std::packaged_task<returnType()> >(
        std::bind(std::forward<Function>(f), std::forward<Args>(args)...)
    );

    m_taskFunc = std::bind(std::forward<Function>(f), std::forward<Args>(args)...);

    return pt;
}

} //namespace vtf