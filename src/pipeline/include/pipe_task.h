/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-30 17:45:25
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-11-06 20:08:41
 */
#pragma once

#include <cflow/common/task.h>
#include <cflow/utils/buffer_manager.h>
#include <cflow/utils/buffer_manager_factory.h>
#include "pipenode_dispatcher.h"
#include "type.h"

using namespace cflow::utils::memory;

namespace cflow::pipeline {
/**
 * @name: class PipeTask
 * @Descripttion: it is a sample code for user. just default implementation for
 * Task class. users can use it directly, and if the user wants to do some
 * customization, can inherit it and override some interfaces. WARNING: user
 * can't change this class.
 * @param {*}
 * @return {*}
 */
class PipeTask : public Task
{
public:
    enum DependencyStatus
    {
        NOREADY = 0,
        READY,
        DONE
    };

    struct DependencyNodeInfo
    {
        cflow_id_t                       nodeId;
        DependencyStatus                 status;
        std::vector<BufferSpecification> input;
        std::vector<BufferSpecification> output;
    };

    using DependencyNodeInfoSP = std::shared_ptr<DependencyNodeInfo>;
    struct Dependency
    {
        cflow_id_t                                  curNodeId = -1;
        std::pair<cflow_id_t, DependencyNodeInfoSP> precursors;
        std::pair<cflow_id_t, DependencyNodeInfoSP> successors;
    };

    using PipeTaskBufferInfoSP =
        std::shared_ptr<BufferManager<void>::BufferInfo>;
    struct NodeBufferInfo
    {
        cflow_id_t                        nodeId;
        std::vector<PipeTaskBufferInfoSP> input;
        std::vector<PipeTaskBufferInfoSP> output;
    };
    using NodeBufferInfoSP = std::shared_ptr<NodeBufferInfo>;

public:
    PipeTask(PipelineScenario scenario, bool enableDebug = false);

    virtual ~PipeTask() {}

    bool constructDependency(
        std::vector<cflow_id_t>&                    pipeline,
        std::shared_ptr<BufferManagerFactory<void>> bufferMgrFactory) override;

    virtual bool constructIO() override;

    PipelineScenario scenario() override { return m_scenario; }

    cflow_id_t getCurrentNode() override { return m_currentProcessNodeId; };

    cflow_id_t getNextNode() override { return m_nextNodeId; };

    bool checkDependencyIsReady() override;

    void markCurrentNodeReady() override;

    void markError() override;

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

    void addNotifierForNode(cflow_id_t nodeId,
                            cflow_id_t notifierId = -1) override;

    std::vector<cflow_id_t> getNotifiersByNodeId(cflow_id_t nodeId) override;

    void addInputForNode(cflow_id_t nodeId, const BufferSpecification&);

    void addOutputForNode(cflow_id_t nodeId, const BufferSpecification&);

    bool setCurrentNodeIO();

    std::vector<PipeTaskBufferInfoSP> input() override
    {
        return m_nodeBufferInfoMap[m_currentProcessNodeId].input;
    }
    std::vector<PipeTaskBufferInfoSP> output() override
    {
        return m_nodeBufferInfoMap[m_currentProcessNodeId].output;
    }

    TaskStatus getStatus() override { return m_status; }

    void skipPipeNode(cflow_id_t nodeId) { m_skipNodeIds.insert(nodeId); }

private:
    bool checkDependencyValid();

    cflow_id_t findNextNode();

    void dumpTaskInfo();

    void releaseCurrentNodeBuffer(bool isInput);

private:
    std::vector<Dependency>                       m_dependencies;
    std::unordered_map<int, DependencyNodeInfoSP> m_dependenciesNodeInfo;
    std::unordered_map<int, NodeBufferInfo>       m_nodeBufferInfoMap;
    std::shared_ptr<BufferManagerFactory<void>>   m_buffeManagerFactory;
    std::unordered_map<cflow_id_t, std::vector<cflow_id_t>> m_nodeNotifiers;
    PipelineScenario                                        m_scenario;
    NotifyStatus                                            m_notifyStatus;
    TaskType                                                m_taskType;
    TaskPriority                                            m_priority;
    cflow_id_t                     m_currentProcessNodeId;
    int                            m_currentProcessNodeIdx;
    cflow_id_t                     m_nextNodeId;
    int                            m_nextNodeIdx;
    bool                           m_enableDebug;
    TaskStatus                     m_status;
    std::unordered_set<cflow_id_t> m_skipNodeIds;
};

} // namespace cflow::pipeline