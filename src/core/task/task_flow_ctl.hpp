/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-02 18:15:32
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-11-06 21:49:02
 */

#pragma once

#include <functional>
#include <future>
#include <memory>
#include <unordered_map>
#include <vector>
#include <fstream>

#include "../dag.hpp"
#include "../type.hpp"
#include "../utils/log/log.hpp"
#include "../utils/dumper.hpp"
#include "tftask.hpp"
#include "task_threadPool.hpp"

#define TASKFLOWCTL_THREADPOOL_MAX_THREAD 8
namespace cflow::task {
class TaskFlowCtl
{
public:
    TaskFlowCtl(bool enableDebug = false)
        : m_threadPool(TASKFLOWCTL_THREADPOOL_MAX_THREAD),
          m_dag(),
          m_bufferMgrFactory(std::make_shared<BufferManagerFactory<void>>()),
          m_debugEnable(enableDebug)
    {
    }

    /**
     * @name: addTask
     * @Descripttion: add a task to task flow
     * @param {*} f mean function object, args is some params.
     * @return {*} a task object
     */
    template <typename Function, typename... Args>
    std::shared_ptr<TFTask> addTask(Function&& f, Args&&... args);

    /**
     * @name: addTaskWithTaskInfo
     * @Descripttion: add task with some infomation
     * @param {*} taskInfo is task infomation
     * @return {*} a task object
     */
    template <typename Function, typename... Args>
    std::shared_ptr<TFTask> addTaskWithTaskInfo(TaskCreateInfo&& taskInfo,
                                                Function&& f, Args&&... args);

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
    void commonSetting(std::shared_ptr<TFTask> task);

private:
    std::unordered_map<cflow_id_t, std::shared_ptr<TFTask>> m_taskIdMap;
    std::vector<std::vector<cflow_id_t>>                    m_taskOrder;
    TaskThreadPool                                          m_threadPool;
    DAG                                                     m_dag;
    std::shared_ptr<BufferManagerFactory<void>>             m_bufferMgrFactory;

    bool m_debugEnable;
};

template <typename Function, typename... Args>
std::shared_ptr<TFTask> TaskFlowCtl::addTask(Function&& f, Args&&... args)
{
    std::shared_ptr<TFTask> task = std::make_shared<TFTask>(m_bufferMgrFactory);
    task->setProcessFunc(std::forward<Function>(f),
                         std::forward<Args>(args)...);
    commonSetting(task);
    return task;
}

template <typename Function, typename... Args>
std::shared_ptr<TFTask> TaskFlowCtl::addTaskWithTaskInfo(
    TaskCreateInfo&& taskInfo, Function&& f, Args&&... args)
{
    std::shared_ptr<TFTask> task =
        std::make_shared<TFTask>(std::move(taskInfo), m_bufferMgrFactory);
    task->setProcessFunc(std::forward<Function>(f),
                         std::forward<Args>(args)...);
    commonSetting(task);
    return task;
}

void TaskFlowCtl::reorganizeTaskOrder()
{
    m_taskOrder = m_dag.topologicalSort();
    if (!m_debugEnable) return;
    {
        uint32_t curSceneraio = 0;
        std::unordered_map<uint32_t, std::vector<std::string>>
                                           scenario2TaskOrderWithNameMap;
        std::set<std::vector<std::string>> pipelineWithNameSet;
        for (auto&& order : m_taskOrder)
        {
            std::vector<std::string> taskOrderWithName;
            for (auto&& taskID : order)
            {
                std::cout << taskID << " ";
                taskOrderWithName.push_back(m_taskIdMap[taskID]->name());
            }
            std::cout << std::endl;
            scenario2TaskOrderWithNameMap[curSceneraio] = taskOrderWithName;
            curSceneraio++;
        }
        cflow::utils::Dumper dumper("Task_flow_control",
                                    scenario2TaskOrderWithNameMap,
                                    DUMPTYPE::TASKFLOW);
        std::string          filename = "Task flow control.dot";
        std::fstream         fs(filename, std::ios::out | std::ios::trunc);
        dumper.dumpDOT(fs);
    }
}

void TaskFlowCtl::commonSetting(std::shared_ptr<TFTask> task)
{
    m_taskIdMap.emplace(task->ID(), task);
    m_dag.addNode(task);
}

void TaskFlowCtl::start()
{
    reorganizeTaskOrder();
    for (auto& curLevelTaskIds : m_taskOrder)
    {
        std::vector<std::pair<std::string, std::future<void>>> taskfutureList;
        for (cflow_id_t taskId : curLevelTaskIds)
        {
            if (m_taskIdMap.count(taskId) == 0)
            {
                std::cerr << "can't find the task id(" << taskId
                          << ") in taskIdMap, please check it." << std::endl;
                return;
            }
            std::shared_ptr<TFTask> task = m_taskIdMap[taskId];
            taskfutureList.emplace_back(task->name(),
                                        m_threadPool.emplaceTask(task));
        }

        for (std::pair<std::string, std::future<void>>& taskfuturePair :
             taskfutureList)
        {
            taskfuturePair.second.get();
            CFLOW_LOGE("{0} execute complate!", taskfuturePair.first);
        }
    }
}
} // namespace cflow::task