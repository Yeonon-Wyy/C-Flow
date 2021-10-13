/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-02 18:15:32
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-13 22:09:52
 */

#pragma once

#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>
#include <future>

#include "task.hpp"
#include "taskthreadPool.hpp"
#include "Dag.hpp"


#define TASKFLOWCTL_THREADPOOL_MAX_THREAD 8
namespace vtf
{

class TaskFlowCtl
{
public:
    TaskFlowCtl(bool enable = false)
        :m_threadPool(TASKFLOWCTL_THREADPOOL_MAX_THREAD),
         m_dag(),
         m_debugEnable(enable) {}


    template<typename Function, typename... Args>
    std::shared_ptr<Task> addTask(Function&& f, Args&&... args);

    //TODO:明天搞
    std::shared_ptr<Task> addTaskWithName(const std::string& taskName);
    std::shared_ptr<Task> addTaskWithPriority(const std::string& taskName);

    void start();

private:
    void reorganizeTaskOrder();
    void commonSetting(std::shared_ptr<Task> task);
private:
    std::unordered_map<long, std::shared_ptr<Task>> m_taskIdMap;
    std::vector<std::vector<long>> m_taskOrder;
    TaskThreadPool m_threadPool;
    DAG m_dag;

    bool m_debugEnable;
};


template<typename Function, typename... Args>
std::shared_ptr<Task> TaskFlowCtl::addTask(Function&& f, Args&&... args)
{
    std::shared_ptr<Task> task = std::make_shared<Task>();
    task->commit(std::forward<Function>(f), std::forward<Args>(args)...);
    commonSetting(task);
    return task;
}

void TaskFlowCtl::reorganizeTaskOrder()
{
    m_taskOrder = m_dag.topologicalSort();
    
    if (m_debugEnable) {
        std::cout << "dump task order: " << std::endl;
        for (auto& curLevelTask : m_taskOrder) {
            std::cout << "[";
            for (long taskId : curLevelTask) {
                std::cout << taskId << ",";
            }
            std::cout << "],";
        }
        std::cout << std::endl;
    }
}

void TaskFlowCtl::commonSetting(std::shared_ptr<Task> task)
{
    m_taskIdMap.emplace(task->getID(), task);
    m_dag.addNode(task);
}

void TaskFlowCtl::start()
{
    reorganizeTaskOrder();
    for (auto& curLevelTaskIds : m_taskOrder) {
        // std::vector<std::future<void>> taskfutureList;
        std::vector<std::pair<std::string, std::future<void>>> taskfutureList;
        for (long taskId : curLevelTaskIds) {
            if (m_taskIdMap.count(taskId) == 0) {
                std::cerr << "can't find the task id(" << taskId << ") in taskIdMap, please check it." << std::endl;
                return;
            }
            std::shared_ptr<Task> task = m_taskIdMap[taskId];
            taskfutureList.emplace_back(task->getName(), m_threadPool.emplaceTask(task));
        }

        for (std::pair<std::string, std::future<void>> &taskfuturePair : taskfutureList) {
            taskfuturePair.second.get();
            std::cout << taskfuturePair.first << " execute complate!" << std::endl;
        }
    }
}


} //namespace vtf