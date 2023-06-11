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
#include <queue>

#include "tftask.h"

#define TASKFLOWCTL_THREADPOOL_MAX_THREAD 8

namespace cflow::task {

// Task Compare, with task priority
struct TaskComp
{
    bool operator()(std::shared_ptr<TFTask> lhs, std::shared_ptr<TFTask> rhs)
    {
        return lhs->getPriority() > rhs->getPriority();
    }
};

class TaskThreadPool
{
public:
    TaskThreadPool(size_t threadSize);

    /**
     * @name: emplaceTask
     * @Descripttion: add a task to thread pool
     * @param {shared_ptr<Task>} task
     * @return {*}
     */
    auto emplaceTask(std::shared_ptr<TFTask> task) -> std::future<void>;

    ~TaskThreadPool();

private:
    // thread list, we need keep fixed number of threads
    std::vector<std::thread> m_workers;

    // task queue
    std::priority_queue<std::shared_ptr<TFTask>,
                        std::vector<std::shared_ptr<TFTask>>, TaskComp>
        m_tasks;

    // for synchronization
    std::mutex              m_taskMutex;
    std::condition_variable m_taskCV;

    // stop flag
    bool isStop;
};

class TaskFlowCtl
{
public:
    TaskFlowCtl(bool enableDebug = false);

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
} // namespace cflow::task