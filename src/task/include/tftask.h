/*
 * @Descripttion: include Task class
 * @version: V1.0
 * @Author: yeonon
 * @Date: 2021-09-22 21:36:41
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-11-06 20:48:23
 */
#pragma once

#include <functional>
#include <future>
#include <initializer_list>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <cflow/common/task.h>
#include <cflow/common/dag.h>

#include "type.h"

#define TASK_NAME_PREFIX "task_"

namespace cflow::task {

class TFTask : public cflow::Task, public DAGNode
{
public:
    TFTask(std::shared_ptr<BufferManagerFactory<void>> bufferMgrFactory);

    TFTask(TaskCreateInfo&&                            createInfo,
           std::shared_ptr<BufferManagerFactory<void>> bufferMgrFactory);

    virtual ~TFTask() {}

    bool constructDependency(
        std::vector<cflow_id_t>&                    taskOrder,
        std::shared_ptr<BufferManagerFactory<void>> bufferMgrFactory) override
    {
        UNUSED(taskOrder);
        UNUSED(bufferMgrFactory);
        return false;
    }
    virtual bool constructIO() override { return false; }
    cflow_id_t   getCurrentNode() override { return -1; };
    cflow_id_t   getNextNode() override { return -1; };
    bool         checkDependencyIsReady() override { return false; };
    void         markCurrentNodeReady() override { return; };
    void         markError() override{};

    TFTaskScenario scenario() override { return m_scenario; }

    void setNotifyStatus(NotifyStatus&& status) override
    {
        m_notifyStatus = std::move(status);
    };

    NotifyStatus getNotifyStatus() override { return m_notifyStatus; }

    void setTaskType(TaskType&& taskType) override
    {
        m_taskType = std::move(taskType);
    }

    TaskType getTaskType() override { return m_taskType; }

    void setPriority(TaskPriority&& priority) override
    {
        m_priority = std::move(priority);
    }

    TaskPriority getPriority() override { return m_priority; }

    void addNotifierForNode(cflow_id_t notifierId, cflow_id_t nodeId) override
    {
        UNUSED(nodeId);
        m_notifiers.push_back(notifierId);
    }

    std::vector<cflow_id_t> getNotifiersByNodeId(cflow_id_t nodeId) override
    {
        return m_notifiers;
    };

    void addInputForNode(const BufferSpecification&);

    void addOutputForNode(const BufferSpecification&);

    std::vector<TFTaskBufferInfoSP> input() override
    {
        return m_nodeBufferInfo.input;
    }
    std::vector<TFTaskBufferInfoSP> output() override
    {
        return m_nodeBufferInfo.output;
    }

    TaskStatus getStatus() override { return m_status; }

    /**
     * @name: commit
     * @Descripttion: commit a task, but not execute task for now.
     * @param {*}
     */
    template <typename Function, typename... Args>
    void setProcessFunc(Function&& f, Args&&... args);

    TFTaskFunc getProcessFunc() { return m_processFunc; }

    std::string name() { return m_name; }

    /**
     * @name: will execute runanble function, must be set
     * @Descripttion:
     * @param {*}
     * @return {*}
     */
    void operator()();

private:
    TFTaskBufferInfoSP getBufferInfoByBFS(const BufferSpecification& bfs);

private:
    std::string                                 m_name; // task name
    TFTaskFunc                                  m_processFunc;
    TaskPriority                                m_priority; // priority
    TFTaskScenario                              m_scenario;
    NotifyStatus                                m_notifyStatus;
    std::vector<cflow_id_t>                     m_notifiers;
    TaskType                                    m_taskType;
    TaskStatus                                  m_status;
    NodeBufferInfo                              m_nodeBufferInfo;
    std::shared_ptr<BufferManagerFactory<void>> m_buffeManagerFactory;
};

template <typename Function, typename... Args>
void TFTask::setProcessFunc(Function&& f, Args&&... args)
{
    m_processFunc =
        std::bind(std::forward<Function>(f), std::forward<Args>(args)...);
}

} // namespace cflow::task