/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-02 18:15:32
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-30 19:04:13
 */

#pragma once

#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>
#include <future>

#include "task.hpp"
#include "taskthreadPool.hpp"
#include "../dag.hpp"
#include "../log.hpp"


#define TASKFLOWCTL_THREADPOOL_MAX_THREAD 8
namespace vtf
{

class TaskFlowCtl
{
public:
    TaskFlowCtl(bool enableDebug = false)
        :m_threadPool(TASKFLOWCTL_THREADPOOL_MAX_THREAD),
         m_dag(),
         m_debugEnable(enableDebug) {}


    /**
     * @name: addTask
     * @Descripttion: add a task to task flow
     * @param {*} f mean function object, args is some params.
     * @return {*} a task object
     */    
    template<typename Function, typename... Args>
    std::shared_ptr<Task> addTask(Function&& f, Args&&... args);

    /**
     * @name: addTaskWithTaskInfo
     * @Descripttion: add task with some infomation
     * @param {*} taskInfo is task infomation
     * @return {*} a task object
     */    
    template<typename Function, typename... Args>
    std::shared_ptr<Task> addTaskWithTaskInfo(TaskCreateInfo&& taskInfo, Function&& f, Args&&... args);

    /**
     * @name: start
     * @Descripttion: start all task according to DAG
     * @param {*} none
     * @return {*} none
     */    
    void start();

private:

    /**
     * @name: reorganizeTaskOrder
     * @Descripttion: reorganize task order, only after add or delete task
     * @param {*} none
     * @return {*} none
     */    
    void reorganizeTaskOrder();

    /**
     * @name: commonSetting
     * @Descripttion: set some common setting for task
     * @param {shared_ptr<Task>} task
     * @return {*}
     */    
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

template<typename Function, typename... Args>
std::shared_ptr<Task> TaskFlowCtl::addTaskWithTaskInfo(TaskCreateInfo&& taskInfo, Function&& f, Args&&... args)
{
    std::shared_ptr<Task> task = std::make_shared<Task>(std::move(taskInfo));
    task->commit(std::forward<Function>(f), std::forward<Args>(args)...);
    commonSetting(task);
    return task;
}

void TaskFlowCtl::reorganizeTaskOrder()
{
    m_taskOrder = m_dag.topologicalSort();
    
    if (m_debugEnable) {
        VTF_LOGI("dump task order: ");
        std::stringstream ss;
        for (auto& curLevelTask : m_taskOrder) {
            ss << "[";
            for (long taskId : curLevelTask) {
                ss << m_taskIdMap[taskId]->getName() << ",";
            }
            VTF_LOGI("{0}]", ss.str());
        }
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
            VTF_LOGI("{0} execute complate!", taskfuturePair.first);
        }
    }
}


} //namespace vtf