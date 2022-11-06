/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-30 17:45:25
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-11-06 19:26:52
 */
#pragma once

#include "../task.hpp"
#include "../utils/id_generator.hpp"
#include "../utils/memory/buffer_manager.hpp"
#include "../utils/memory/buffer_manager_factory.hpp"
#include "../utils/str_convertor.hpp"
#include "pipenode_dispatcher.hpp"
#include "type.hpp"

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

    void addNotifierForNode(cflow_id_t nodeId, cflow_id_t notifierId) override;

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

PipeTask::PipeTask(PipelineScenario scenario, bool enableDebug)
    : Task(),
      m_scenario(scenario),
      m_notifyStatus(NotifyStatus::OK),
      m_taskType(TaskType::NORMAL),
      m_priority(TaskPriority::NORMAL),
      m_currentProcessNodeId(-1),
      m_currentProcessNodeIdx(-1),
      m_nextNodeId(-1),
      m_nextNodeIdx(-1),
      m_enableDebug(enableDebug),
      m_status(TaskStatus::OK)
{
}

bool PipeTask::constructDependency(
    std::vector<cflow_id_t>&                    pipeline,
    std::shared_ptr<BufferManagerFactory<void>> bufferMgrFactory)
{
    m_dependencies.clear();

    // skip nodes, if need
    for (std::vector<cflow_id_t>::iterator it = pipeline.begin();
         it != pipeline.end();)
    {
        if (m_skipNodeIds.count(*it))
        {
            it = pipeline.erase(it);
        }
        else
        {
            it++;
        }
    }

    auto constructDependencyNodeInfo = [&](cflow_id_t       nodeId,
                                           DependencyStatus status) {
        auto nodeInfoSp    = std::make_shared<DependencyNodeInfo>();
        nodeInfoSp->nodeId = nodeId;
        nodeInfoSp->status = status;
        return nodeInfoSp;
    };

    auto constructNodeInfo = [&pipeline, &constructDependencyNodeInfo, this]() {
        for (size_t i = 0; i < pipeline.size(); i++)
        {
            auto nodeInfo = constructDependencyNodeInfo(
                pipeline[i], DependencyStatus::NOREADY);
            this->m_dependenciesNodeInfo[nodeInfo->nodeId] = nodeInfo;
        }
        // construct a empty node as dummy node
        auto dummyNode =
            constructDependencyNodeInfo(-1, DependencyStatus::DONE);
        this->m_dependenciesNodeInfo[-1] = dummyNode;
    };

    auto constructNodeBufferInfo = [&pipeline, this]() {
        for (size_t i = 0; i < pipeline.size(); i++)
        {
            auto nodeBufferInfo = NodeBufferInfo{.nodeId = pipeline[i]};
            m_nodeBufferInfoMap[nodeBufferInfo.nodeId] = nodeBufferInfo;
        }
    };

    // construct all node info bt pipeline
    constructNodeInfo();

    // construct all node buffer map
    constructNodeBufferInfo();

    for (size_t i = 0; i < pipeline.size(); i++)
    {
        cflow_id_t curNodeId = pipeline[i];

        // check dependency exist
        auto it = std::find_if(m_dependencies.begin(), m_dependencies.end(),
                               [curNodeId](const Dependency& dependency) {
                                   return dependency.curNodeId == curNodeId;
                               });
        if (it != m_dependencies.end())
        {
            CFLOW_LOGE(
                "already exist dependency of node {0}, please check it first.",
                curNodeId);
            m_dependencies.clear();
        }

        Dependency dependency = Dependency{.curNodeId = curNodeId};
        if (i >= 1)
        {
            dependency.precursors = {pipeline[i - 1],
                                     m_dependenciesNodeInfo[pipeline[i - 1]]};
        }
        else
        {
            dependency.precursors = {-1, m_dependenciesNodeInfo[-1]};
        }
        if (i + 1 < pipeline.size())
        {
            dependency.successors = {pipeline[i + 1],
                                     m_dependenciesNodeInfo[pipeline[i + 1]]};
        }
        else
        {
            dependency.successors = {-1, m_dependenciesNodeInfo[-1]};
        }
        m_dependencies.push_back(dependency);
    }

    if (m_dependencies.empty())
    {
        CFLOW_LOGE("dependency size of pipe task can't be less than 1");
        return false;
    }

    // set current process node to first node
    m_currentProcessNodeId  = m_dependencies[0].curNodeId;
    m_currentProcessNodeIdx = 0;
    m_nextNodeIdx           = m_currentProcessNodeIdx + 1;
    m_nextNodeId            = findNextNode();

    m_buffeManagerFactory = bufferMgrFactory;
    // construct input and output
    constructIO();

    dumpTaskInfo();

    return true;
}

bool PipeTask::constructIO()
{
    // noting need to do by default
    return true;
}

cflow_id_t PipeTask::findNextNode()
{
    if (!checkDependencyValid()) return -1;
    Dependency currentDependency = m_dependencies[m_currentProcessNodeIdx];
    cflow_id_t successorId       = currentDependency.successors.first;
    DependencyStatus successorStatus =
        currentDependency.successors.second->status;
    if (successorId != -1 && successorStatus == DependencyStatus::NOREADY)
    {
        return successorId;
    }
    return -1;
}

bool PipeTask::checkDependencyIsReady()
{
    if (!checkDependencyValid()) return false;
    Dependency currentDependency = m_dependencies[m_currentProcessNodeIdx];
    cflow_id_t precursorId       = currentDependency.precursors.first;
    DependencyStatus precursorStatus =
        currentDependency.precursors.second->status;

    if ((precursorId == -1 && precursorStatus == DependencyStatus::DONE) ||
        (precursorId != -1 && precursorStatus == DependencyStatus::READY))
    {
        CFLOW_LOGD("precursor node [{0}] is ready", precursorId);
        return true;
    }
    return false;
}

void PipeTask::markCurrentNodeReady()
{
    cflow_id_t nextNodeId = findNextNode();
    m_nextNodeIdx         = m_currentProcessNodeIdx + 1;
    // last node
    if (nextNodeId == -1 || m_nextNodeIdx >= (int)m_dependencies.size())
    {
        CFLOW_LOGD("task {0} node [{1}] have done.", ID(),
                   m_currentProcessNodeId);
        releaseCurrentNodeBuffer(true);
        releaseCurrentNodeBuffer(false);
        m_currentProcessNodeId = -1;
        m_currentProcessNodeIdx++;
        m_nextNodeId  = -1;
        m_nextNodeIdx = -1;
        CFLOW_LOGD("task {0} all node already process done.", ID());
        return;
    }

    if (m_dependencies[m_nextNodeIdx].precursors.first !=
        m_dependencies[m_currentProcessNodeIdx].curNodeId)
    {
        CFLOW_LOGD(
            "node [{0}] and node [{1}] no connection. please check dependency.",
            m_dependencies[m_currentProcessNodeIdx].curNodeId,
            m_dependencies[m_nextNodeIdx].precursors.first);
        return;
    }

    // mark next node denpendency's pre is done
    m_dependencies[m_nextNodeIdx].precursors.second->status =
        DependencyStatus::READY;
    CFLOW_LOGD("task {0} node [{1}] have done. ", ID(), m_currentProcessNodeId);
    releaseCurrentNodeBuffer(true);
    m_currentProcessNodeId = nextNodeId;
    m_currentProcessNodeIdx++;
    m_nextNodeId = findNextNode();
}

void PipeTask::markError()
{
    // release current node buffer, inlcude input and output
    releaseCurrentNodeBuffer(true);
    releaseCurrentNodeBuffer(false);
    // reset task control info
    m_currentProcessNodeId = -1;
    m_currentProcessNodeIdx++;
    m_nextNodeId  = -1;
    m_nextNodeIdx = -1;
    // mark status is error
    m_status = TaskStatus::ERROR;
    CFLOW_LOGD("task {0} is error.", ID());
}

void PipeTask::addNotifierForNode(cflow_id_t nodeId, cflow_id_t notifierId)
{
    m_nodeNotifiers[nodeId].push_back(notifierId);
}

std::vector<cflow_id_t> PipeTask::getNotifiersByNodeId(cflow_id_t nodeId)
{
    if (m_nodeNotifiers.count(nodeId) > 0)
    {
        return m_nodeNotifiers[nodeId];
    }
    return {};
}

// private
bool PipeTask::checkDependencyValid()
{
    if (m_currentProcessNodeIdx < 0 ||
        m_currentProcessNodeIdx >= (int)m_dependencies.size())
    {
        CFLOW_LOGE("current process node index {0} is error. please check it.",
                   m_currentProcessNodeIdx);
        return false;
    }
    Dependency currentDependency = m_dependencies[m_currentProcessNodeIdx];
    if (currentDependency.curNodeId != m_currentProcessNodeId)
    {
        CFLOW_LOGE("current process node dependency's node [{0}] must equal "
                   "current process node [{1}].",
                   currentDependency.curNodeId, m_currentProcessNodeId);
        return false;
    }
    return true;
}

void PipeTask::addInputForNode(cflow_id_t                 nodeId,
                               const BufferSpecification& bfs)
{
    if (m_dependenciesNodeInfo.count(nodeId) == 0)
    {
        CFLOW_LOGE("can't find node{0} in task path", nodeId);
    }
    auto nodeInfo = m_dependenciesNodeInfo[nodeId];
    if (!m_buffeManagerFactory->getBufferManager(bfs))
    {
        m_buffeManagerFactory->createBufferManager(bfs);
    }
    nodeInfo->input.push_back(bfs);
    CFLOW_LOGD("node{0} add input buffer{1} success!", nodeId, bfs.name);
}

void PipeTask::addOutputForNode(cflow_id_t                 nodeId,
                                const BufferSpecification& bfs)
{
    if (m_dependenciesNodeInfo.count(nodeId) == 0)
    {
        CFLOW_LOGE("can't find node{0} in task path", nodeId, bfs.name);
    }
    auto nodeInfo = m_dependenciesNodeInfo[nodeId];
    if (!m_buffeManagerFactory->getBufferManager(bfs))
    {
        m_buffeManagerFactory->createBufferManager(bfs);
    }
    nodeInfo->output.push_back(bfs);
    CFLOW_LOGD("node{0} add output buffer{1} success!", nodeId, bfs.name);
}

bool PipeTask::setCurrentNodeIO()
{
    bool ret = true;

    auto currentNodeBufferInfo =
        m_nodeBufferInfoMap.find(m_currentProcessNodeId);
    auto currentDependency = m_dependenciesNodeInfo[m_currentProcessNodeId];

    // get output buffer
    for (auto&& outputBFS : currentDependency->output)
    {
        auto bufMgr = m_buffeManagerFactory->getBufferManager(outputBFS);
        if (!bufMgr)
        {
            bufMgr = m_buffeManagerFactory->createBufferManager(outputBFS);
        }
        auto bufInfo = bufMgr->popBuffer();
        currentNodeBufferInfo->second.output.push_back(bufInfo);
        CFLOW_LOGD("set task {0} output buffer {1} for current node {2}", ID(),
                   bufInfo->name, m_currentProcessNodeId);
        // not last
        if (m_nextNodeId > 0)
        {
            // input of current node is same as output of next node.
            m_nodeBufferInfoMap[m_nextNodeId].input.push_back(bufInfo);
            CFLOW_LOGD("set task {0} input buffer {1} for next node {2}", ID(),
                       bufInfo->name, m_nextNodeId);
        }
    }

    return ret;
}

void PipeTask::releaseCurrentNodeBuffer(bool isInput)
{
    auto currentDependency     = m_dependenciesNodeInfo[m_currentProcessNodeId];
    auto currentNodeBufferInfo = m_nodeBufferInfoMap[m_currentProcessNodeId];
    if (isInput)
    {
        // input
        for (auto&& inputBFS : currentDependency->input)
        {
            auto bufMgr = m_buffeManagerFactory->getBufferManager(inputBFS);
            if (!bufMgr)
            {
                CFLOW_LOGE("error!! can't find buffer manager {0}. can't "
                           "release buffer",
                           inputBFS.name);
                continue;
            }

            auto it = std::find_if(currentNodeBufferInfo.input.begin(),
                                   currentNodeBufferInfo.input.end(),
                                   [&inputBFS](PipeTaskBufferInfoSP& bufInfo) {
                                       return bufInfo->name == inputBFS.name;
                                   });
            CFLOW_LOGD("currentNodeBufferInfo input size() = {0}",
                       currentNodeBufferInfo.input.size());
            if (it == currentNodeBufferInfo.input.end())
            {
                CFLOW_LOGE("error!! can't find buffer info {0}", inputBFS.name);
                continue;
            }
            bufMgr->pushBuffer(*it);
            CFLOW_LOGD("release {0} input buffer {1} of node {2}", ID(),
                       inputBFS.name, m_currentProcessNodeId);
        }
    }
    else
    {
        // output
        for (auto&& outputBFS : currentDependency->output)
        {
            auto bufMgr = m_buffeManagerFactory->getBufferManager(outputBFS);
            if (!bufMgr)
            {
                CFLOW_LOGE("error!! can't find buffer manager {0}. can't "
                           "release buffer",
                           outputBFS.name);
                continue;
            }

            auto it = std::find_if(currentNodeBufferInfo.output.begin(),
                                   currentNodeBufferInfo.output.end(),
                                   [&outputBFS](PipeTaskBufferInfoSP& bufInfo) {
                                       return bufInfo->name == outputBFS.name;
                                   });
            if (it == currentNodeBufferInfo.output.end())
            {
                CFLOW_LOGE("error!! can't find buffer info {0}",
                           outputBFS.name);
                continue;
            }
            bufMgr->pushBuffer(*it);
            CFLOW_LOGD("release {0} output buffer {1} of node {2}", ID(),
                       outputBFS.name, m_currentProcessNodeId);
        }
    }
}

void PipeTask::dumpTaskInfo()
{
    // dump
    std::string str = "";

    if (m_enableDebug)
    {
        for (auto& dependency : m_dependencies)
        {
            CFLOW_LOGD("node : [{0}]", dependency.curNodeId);

            CFLOW_LOGD("pre : [{0}]", dependency.precursors.first);
            str.clear();
            str += "\tinput : ";
            for (auto&& bfs : dependency.precursors.second->input)
            {
                str += "[" + bfs.name + "],";
            }
            str.pop_back();
            CFLOW_LOGD(str);
            str.clear();
            str += "\toutput : ";
            for (auto&& bfs : dependency.precursors.second->output)
            {
                str += "[" + bfs.name + "],";
            }
            str.pop_back();
            CFLOW_LOGD(str);
            CFLOW_LOGD("suc : [{0}]", dependency.successors.first);

            str.clear();
            str += "\tinput : ";
            for (auto&& bfs : dependency.successors.second->input)
            {
                str += "[" + bfs.name + "],";
            }
            str.pop_back();
            CFLOW_LOGD(str);
            str.clear();
            str += "\toutput : ";
            for (auto&& bfs : dependency.successors.second->output)
            {
                str += "[" + bfs.name + "],";
            }
            str.pop_back();
            CFLOW_LOGD(str);
            str.clear();
            CFLOW_LOGD("--------------------------------------");
        }
    }
}

} // namespace cflow::pipeline