/*
 * @Descripttion: include Task class
 * @version: V1.0
 * @Author: yeonon
 * @Date: 2021-09-22 21:36:41
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-02 17:37:07
 */
#pragma once

#include <functional>
#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include <initializer_list>

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

template<typename ReturnType>
class Task : public DAGNode {
public:

    /**
     * @name: Task
     * @Descripttion: Task templeate constructor
     * @param {*} f is function object, args is agr list
     * @return {*}
     */
    template<typename Function, typename... Args>
    explicit Task(Function&& f, Args&&... args);

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

    //TODO: I think shouldn't return the function object to other module
    /**
     * @name: getExecFunc
     * @Descripttion: return a function object
     * @param {*}
     * @return {*}
     */    
    std::function<ReturnType()> getExecFunc() { return m_execFunc; }

private:
    std::function<ReturnType()> m_execFunc;              //function object
    std::string m_name;                                  //task name
    long m_ID;                                           //task id
};

//Note: 类模板里如果有模板成员函数，并且需要在类外定义，要加上类的模板参数
template<typename ReturnType>
template<typename Function, typename... Args>
Task<ReturnType>::Task(Function&& f, Args&&... args) 
    :DAGNode(IDGenerator::getInstance()->generate())
{
    m_execFunc = std::bind(std::forward<Function>(f), std::forward<Args>(args)...);    
    std::stringstream ss;
    ss << TASK_NAME_PREFIX << m_ID;
    ss >> m_name;
}

} //namespace vtf