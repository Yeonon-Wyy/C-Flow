/*
 * @Descripttion: include Task class
 * @version: V1.0
 * @Author: yeonon
 * @Date: 2021-09-22 21:36:41
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-09-04 19:21:00
 */
#pragma once

#include <functional>
#include <future>
#include <initializer_list>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "../dag.hpp"
#include "../utils/id_generator.hpp"
#include "../utils/log/log.hpp"
#include "../utils/str_convertor.hpp"

#define TASK_NAME_PREFIX "task_"

namespace cflow::task {
enum TaskPriority
{
    NOURGENCY,
    NORMAL,
    URGENCY,
    EXTREME_URGENCY,
};

struct TaskCreateInfo
{
    int priority = TaskPriority::NORMAL;
    std::string name = "";
};

class Task : public DAGNode
{
public:
    Task()
        : DAGNode(m_idGenerator.generate()),
          m_ID(getNodeId()),
          m_priority(TaskPriority::NORMAL)
    {
        m_name = TASK_NAME_PREFIX + utils::StringConvetor::digit2String(m_ID);
    }

    Task(TaskCreateInfo&& createInfo)
        : DAGNode(m_idGenerator.generate()),
          m_ID(getNodeId()),
          m_priority(createInfo.priority)
    {
        if (createInfo.name == "")
        {
            m_name =
                TASK_NAME_PREFIX + utils::StringConvetor::digit2String(m_ID);
        }
        else
        {
            m_name = std::move(createInfo.name);
        }
    }

    /**
     * @name: commit
     * @Descripttion: commit a task, but not execute task for now. just
     * construct a package_task object than retrun it.
     * @param {*}
     * @return {*} a package_task object
     */
    template <typename Function, typename... Args>
    auto commit(Function&& f, Args&&... args) -> std::shared_ptr<
        std::packaged_task<typename std::result_of<Function(Args...)>::type()>>;

    /* setter and getter function  */
    Task& setName(const std::string& name)
    {
        m_name = std::move(name);
        return *this;
    }
    std::string getName() { return m_name; }

    long getID() { return m_ID; }

    Task& setPriority(int priority)
    {
        m_priority = priority;
        return *this;
    }
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
    Task& setRunable(std::function<void()>&& runable)
    {
        m_runable = std::move(runable);
        return *this;
    }

    /**
     * @name: will execute runanble function, must be set
     * @Descripttion:
     * @param {*}
     * @return {*}
     */
    void operator()()
    {
        if (m_runable)
        {
            m_runable();
        }
        else
        {
            throw std::runtime_error("before execute, must set runable");
            ;
        }
    }

private:
    static utils::IDGenerator m_idGenerator;
    std::string m_name;               // task name
    long m_ID;                        // task id
    std::function<void()> m_taskFunc; // task function, will execute user task
    int m_priority;                   // priority
    std::function<void()> m_runable;  // runable function, only for threadPool
};

utils::IDGenerator Task::m_idGenerator;

template <typename Function, typename... Args>
auto Task::commit(Function&& f, Args&&... args) -> std::shared_ptr<
    std::packaged_task<typename std::result_of<Function(Args...)>::type()>>
{
    using returnType = typename std::result_of<Function(Args...)>::type;

    auto pt = std::make_shared<std::packaged_task<returnType()>>(
        std::bind(std::forward<Function>(f), std::forward<Args>(args)...));

    m_taskFunc =
        std::bind(std::forward<Function>(f), std::forward<Args>(args)...);

    return pt;
}
} // namespace cflow::task