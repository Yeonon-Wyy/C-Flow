#pragma once

#include <functional>
#include <string>
#include "utils.hpp"
#include <sstream>
#include <memory>
#include <vector>
#include <initializer_list>

#include "Dag.hpp"

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
    template<typename Function, typename... Args>
    explicit Task(Function&& f, Args&&... args);

    //utils function
    
    void setName(const std::string& name) { m_name = std::move(name); }
    std::string getName() { return m_name; }

    long getID() { return m_ID; }

    void setPriority(TaskPriority priority) { m_priority = priority; }
    TaskPriority getPriority() { return m_priority; }

    std::function<ReturnType()> getExecFunc() { return m_execFunc; }

    // Task& precede(std::initializer_list<Task> tasks);

    // Task& succeed(std::initializer_list<Task> tasks);

private:
    std::function<ReturnType()> m_execFunc;
    std::string m_name;
    long m_ID;
    TaskPriority m_priority;

    std::vector<std::shared_ptr<Task>> m_precedes;
    std::vector<std::shared_ptr<Task>> m_succeeds;
};

//Note: 类模板里如果有模板成员函数，并且需要在类外定义，要加上类的模板参数
template<typename ReturnType>
template<typename Function, typename... Args>
Task<ReturnType>::Task(Function&& f, Args&&... args) 
    :DAGNode(IDGenerator::getInstance()->generate()),
    m_priority(TaskPriority::NORMAL)
{
    m_execFunc = std::bind(std::forward<Function>(f), std::forward<Args>(args)...);
    // m_ID = IDGenerator::getInstance()->generate();
    
    std::stringstream ss;
    ss << TASK_NAME_PREFIX << m_ID;
    ss >> m_name;
}

// template<typename ReturnType>
// Task<ReturnType>& Task<ReturnType>::precede(std::initializer_list<Task> tasks)
// {

//     for (Task& task : tasks) {
//         m_precedes.push_back(std::make_shared<Task>(task));
//     }
//     return *this;
// }

// template<typename ReturnType>
// Task<ReturnType>& Task<ReturnType>::succeed(std::initializer_list<Task> tasks)
// {
//     for (Task& task : tasks) {
//         m_succeeds.push_back(std::make_shared<Task>(task));
//     }
//     return *this;
// }

}