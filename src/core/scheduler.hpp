/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2022-01-22 20:06:27
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-10-11 22:27:53
 */
#pragma once

#include <cstdlib>
#include <map>
#include <queue>
#include <unordered_map>

#include "type.hpp"
#include "utils/log/log.hpp"

namespace cflow {
/**
 * @name: class Scheduler
 * @Descripttion: Scheduler can schedule item according to priority(task type).
 *                1. taskType is both task's type and priority
 *                2. when call schedule, it will foreach queue from low to high
 * according to priority.
 * @param {*}
 * @return {*}
 */
template <typename T>
class Scheduler
{
public:
    struct ItemPriorityComp
    {
        template <typename Q = T>
        typename std::enable_if<std::is_pointer<Q>::value ||
                                    cflow::is_shared_ptr<Q>::value,
                                bool>::type
        operator()(T lhs, T rhs)
        {
            return lhs->getPriority() < rhs->getPriority();
        }

        template <typename Q = T>
        typename std::enable_if<!std::is_pointer<Q>::value &&
                                    !cflow::is_shared_ptr<Q>::value,
                                bool>::type
        operator()(T lhs, T rhs)
        {
            return lhs.getPriority() < rhs.getPriority();
            ;
        }
    };

    using SchedulerQueue =
        std::priority_queue<T, std::vector<T>, ItemPriorityComp>;

public:
    Scheduler();

    void emplace(T item);
    T schedule();

    bool empty();
    size_t getQueueCapWithFromItem(T item);
    size_t getQueueSizeWithFromItem(T item);

private:
    template <typename Q = T>
    typename std::enable_if<std::is_pointer<Q>::value ||
                                cflow::is_shared_ptr<Q>::value,
                            TaskType>::type
    extractTaskTypeFromItem(T item)
    {
        return item->getTaskType();
    }

    template <typename Q = T>
    typename std::enable_if<!std::is_pointer<Q>::value &&
                                !cflow::is_shared_ptr<Q>::value,
                            TaskType>::type
    extractTaskTypeFromItem(T item)
    {
        return item.getTaskType();
    }

private:
    std::map<TaskType, SchedulerQueue> m_taskTypeQueueMap;
    std::unordered_map<TaskType, size_t> m_taskTypeQueueCapMap;
};

// ---------------- Scheduler begin----------------

template <typename T>
Scheduler<T>::Scheduler()
{
    m_taskTypeQueueMap[TaskType::taskTYPE_RT] = SchedulerQueue();
    m_taskTypeQueueCapMap[TaskType::taskTYPE_RT] = RT_TASK_CAPCITY;
    m_taskTypeQueueMap[TaskType::taskTYPE_NORMAL] = SchedulerQueue();
    m_taskTypeQueueCapMap[TaskType::taskTYPE_NORMAL] = NORMAL_TASK_CAPCITY;
    m_taskTypeQueueMap[TaskType::taskTYPE_IDEL] = SchedulerQueue();
    m_taskTypeQueueCapMap[TaskType::taskTYPE_IDEL] = IDEL_TASK_CAPCITY;
}

template <typename T>
void Scheduler<T>::emplace(T item)
{
    auto curItemTaskType = extractTaskTypeFromItem(item);
    if (curItemTaskType <= TaskType::taskTYPE_START ||
        curItemTaskType >= TaskType::taskTYPE_END)
    {
        CFLOW_LOGE("please check current task's task type (%d)",
                   curItemTaskType);
        std::abort();
    }
    m_taskTypeQueueMap[curItemTaskType].push(item);
}

template <typename T>
T Scheduler<T>::schedule()
{
    T item;

    // has priority
    for (auto& [taskType, taskQueue] : m_taskTypeQueueMap)
    {
        if (taskQueue.empty()) continue;
        item = taskQueue.top();
        taskQueue.pop();
        break;
    }
    return item;
}

template <typename T>
bool Scheduler<T>::empty()
{
    size_t curSz = 0;
    for (auto& [taskType, itemQueue] : m_taskTypeQueueMap)
    {
        curSz += itemQueue.size();
    }
    return curSz == 0;
}

template <typename T>
size_t Scheduler<T>::getQueueCapWithFromItem(T item)
{
    auto curItemTaskType = extractTaskTypeFromItem(item);

    if (curItemTaskType <= TaskType::taskTYPE_START ||
        curItemTaskType >= TaskType::taskTYPE_END ||
        m_taskTypeQueueCapMap.count(curItemTaskType) == 0)
    {
        CFLOW_LOGE("please check current task's task type (%d)",
                   curItemTaskType);
        std::abort();
    }
    return m_taskTypeQueueCapMap[curItemTaskType];
}

template <typename T>
size_t Scheduler<T>::getQueueSizeWithFromItem(T item)
{
    auto curItemTaskType = extractTaskTypeFromItem(item);

    if (curItemTaskType <= TaskType::taskTYPE_START ||
        curItemTaskType >= TaskType::taskTYPE_END ||
        m_taskTypeQueueMap.count(curItemTaskType) == 0)
    {
        CFLOW_LOGE("please check current task's task type (%d)",
                   curItemTaskType);
        std::abort();
    }
    return m_taskTypeQueueMap[curItemTaskType].size();
}
// ---------------- Scheduler end----------------

} // namespace cflow