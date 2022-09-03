/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-30 17:45:25
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-08-14 20:58:20
 */
#pragma once

#include "../data.hpp"
#include "../utils/id_generator.hpp"
#include "../utils/memory/buffer_manager.hpp"
#include "../utils/memory/buffer_manager_factory.hpp"
#include "../utils/str_convertor.hpp"
#include "pipenode_dispatcher.hpp"
#include "type.hpp"

using namespace cflow::utils::memory;

namespace cflow
{
namespace pipeline
{
/**
 * @name: class PipeData
 * @Descripttion: it is a sample code for user. just default implementation for Data class.
 *                users can use it directly, and if the user wants to do some customization, can inherit it and override some interfaces.
 *                WARNING: user can't change this class.
 * @param {*}
 * @return {*}
 */
class PipeData : public Data
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
        cflow_id_t                         nodeId;
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

    using PipeDataBufferInfoSP = std::shared_ptr<BufferManager<void>::BufferInfo>;
    struct NodeBufferInfo
    {
        cflow_id_t                          nodeId;
        std::vector<PipeDataBufferInfoSP> input;
        std::vector<PipeDataBufferInfoSP> output;
    };
    using NodeBufferInfoSP = std::shared_ptr<NodeBufferInfo>;

public:
    PipeData(PipelineScenario scenario, bool enableDebug = false);

    ~PipeData() {}

    bool constructDependency(const std::vector<cflow_id_t>& pipeline, std::shared_ptr<BufferManagerFactory<void>> bufferMgrFactory) override;

    virtual bool constructIO() override;

    PipelineScenario scenario() override { return m_scenario; }

    cflow_id_t getCurrentNode() override { return m_currentProcessNodeId; };

    cflow_id_t getNextNode() override { return {m_nextNodeId}; };

    bool checkDependencyIsReady() override;

    void markCurrentNodeReady() override;

    void setNotifyStatus(NotifyStatus&& status) override { m_notifyStatus = std::move(status); };

    NotifyStatus getNotifyStatus() { return m_notifyStatus; }

    void setDataType(DataType&& dataType) { m_dataType = std::move(dataType); }

    DataType getDataType() { return m_dataType; }

    void setPriority(DataPriority&& priority) { m_priority = std::move(priority); }

    DataPriority getPriority() { return m_priority; }

    void addNotifierForNode(cflow_id_t nodeId, cflow_id_t notifierId) override;

    std::vector<cflow_id_t> getNotifiersByNodeId(cflow_id_t nodeId) override;

    void addInputForNode(cflow_id_t nodeId, const BufferSpecification&);

    void addOutputForNode(cflow_id_t nodeId, const BufferSpecification&);

    bool setCurrentNodeIO();

    std::vector<PipeDataBufferInfoSP> input() override { return m_nodeBufferInfoMap[m_currentProcessNodeId].input; }
    std::vector<PipeDataBufferInfoSP> output() override { return m_nodeBufferInfoMap[m_currentProcessNodeId].output; }

private:
    bool checkDependencyValid();

    cflow_id_t findNextNode();

    bool notifyResult();

    void dumpDataInfo();

    void releaseCurrentNodeBuffer(bool isInput);

private:
    std::vector<Dependency>                             m_dependencies;
    std::unordered_map<int, DependencyNodeInfoSP>       m_dependenciesNodeInfo;
    std::unordered_map<int, NodeBufferInfo>             m_nodeBufferInfoMap;
    std::shared_ptr<BufferManagerFactory<void>>         m_buffeManagerFactory;
    std::unordered_map<cflow_id_t, std::vector<cflow_id_t>> m_nodeNotifiers;
    PipelineScenario                                    m_scenario;
    NotifyStatus                                        m_notifyStatus;
    DataType                                            m_dataType;
    DataPriority                                        m_priority;
    cflow_id_t                                            m_currentProcessNodeId;
    int                                                 m_currentProcessNodeIdx;
    cflow_id_t                                            m_nextNodeId;
    int                                                 m_nextNodeIdx;
    bool                                                m_enableDebug;
};

PipeData::PipeData(PipelineScenario scenario, bool enableDebug)
    : Data(),
      m_scenario(scenario),
      m_notifyStatus(NotifyStatus::OK),
      m_dataType(DataType::DATATYPE_NORMAL),
      m_priority(DataPriority::DATAPRIORITY_NORMAL),
      m_currentProcessNodeId(-1),
      m_currentProcessNodeIdx(-1),
      m_nextNodeId(-1),
      m_nextNodeIdx(-1),
      m_enableDebug(enableDebug)
{
}

bool PipeData::constructDependency(const std::vector<cflow_id_t>& pipeline, std::shared_ptr<BufferManagerFactory<void>> bufferMgrFactory)
{
    m_dependencies.clear();

    auto constructDependencyNodeInfo = [&](cflow_id_t nodeId, DependencyStatus status) {
        auto nodeInfoSp    = std::make_shared<DependencyNodeInfo>();
        nodeInfoSp->nodeId = nodeId;
        nodeInfoSp->status = status;
        return nodeInfoSp;
    };

    auto constructNodeInfo = [&pipeline, &constructDependencyNodeInfo, this]() {
        for (size_t i = 0; i < pipeline.size(); i++)
        {
            auto nodeInfo                                  = constructDependencyNodeInfo(pipeline[i], DependencyStatus::NOREADY);
            this->m_dependenciesNodeInfo[nodeInfo->nodeId] = nodeInfo;
        }
        // construct a empty node as dummy node
        auto dummyNode                   = constructDependencyNodeInfo(-1, DependencyStatus::DONE);
        this->m_dependenciesNodeInfo[-1] = dummyNode;
    };

    auto constructNodeBufferInfo = [&pipeline, this]() {
        for (size_t i = 0; i < pipeline.size(); i++)
        {
            auto nodeBufferInfo                        = NodeBufferInfo{.nodeId = pipeline[i]};
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
        auto it = std::find_if(m_dependencies.begin(), m_dependencies.end(), [curNodeId](const Dependency& dependency) { return dependency.curNodeId == curNodeId; });
        if (it != m_dependencies.end())
        {
            CFLOW_LOGE("already exist dependency of node {0}, please check it first.", curNodeId);
            m_dependencies.clear();
        }

        Dependency dependency = Dependency{.curNodeId = curNodeId};
        if (i >= 1)
        {
            dependency.precursors = {pipeline[i - 1], m_dependenciesNodeInfo[pipeline[i - 1]]};
        }
        else
        {
            dependency.precursors = {-1, m_dependenciesNodeInfo[-1]};
        }
        if (i + 1 < pipeline.size())
        {
            dependency.successors = {pipeline[i + 1], m_dependenciesNodeInfo[pipeline[i + 1]]};
        }
        else
        {
            dependency.successors = {-1, m_dependenciesNodeInfo[-1]};
        }
        m_dependencies.push_back(dependency);
    }

    if (m_dependencies.empty())
    {
        CFLOW_LOGE("dependency size of pipe data can't be less than 1");
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

    dumpDataInfo();

    return true;
}

bool PipeData::constructIO()
{
    // noting need to do by default
    return true;
}

cflow_id_t PipeData::findNextNode()
{
    if (!checkDependencyValid()) return -1;
    Dependency       currentDependency = m_dependencies[m_currentProcessNodeIdx];
    cflow_id_t         successorId       = currentDependency.successors.first;
    DependencyStatus successorStatus   = currentDependency.successors.second->status;
    if (successorId != -1 && successorStatus == DependencyStatus::NOREADY)
    {
        return successorId;
    }
    return -1;
}

bool PipeData::checkDependencyIsReady()
{
    if (!checkDependencyValid()) return false;
    Dependency       currentDependency = m_dependencies[m_currentProcessNodeIdx];
    cflow_id_t         precursorId       = currentDependency.precursors.first;
    DependencyStatus precursorStatus   = currentDependency.precursors.second->status;

    if ((precursorId == -1 && precursorStatus == DependencyStatus::DONE) || (precursorId != -1 && precursorStatus == DependencyStatus::READY))
    {
        CFLOW_LOGD("precursor node [{0}] is ready", precursorId);
        return true;
    }
    return false;
}

void PipeData::markCurrentNodeReady()
{
    cflow_id_t nextNodeId = findNextNode();
    m_nextNodeIdx       = m_currentProcessNodeIdx + 1;
    // last node
    if (nextNodeId == -1 || m_nextNodeIdx >= (int)m_dependencies.size())
    {
        CFLOW_LOGD("data {0} node [{1}] have done.", ID(), m_currentProcessNodeId);
        releaseCurrentNodeBuffer(true);
        releaseCurrentNodeBuffer(false);
        m_currentProcessNodeId = -1;
        m_currentProcessNodeIdx++;
        m_nextNodeId  = -1;
        m_nextNodeIdx = -1;
        CFLOW_LOGD("data {0} all node already process done.", ID());
        return;
    }

    if (m_dependencies[m_nextNodeIdx].precursors.first != m_dependencies[m_currentProcessNodeIdx].curNodeId)
    {
        CFLOW_LOGD("node [{0}] and node [{1}] no connection. please check dependency.", m_dependencies[m_currentProcessNodeIdx].curNodeId, m_dependencies[m_nextNodeIdx].precursors.first);
        return;
    }

    // mark next node denpendency's pre is done
    m_dependencies[m_nextNodeIdx].precursors.second->status = DependencyStatus::READY;
    CFLOW_LOGD("data {0} node [{1}] have done. ", ID(), m_currentProcessNodeId);
    releaseCurrentNodeBuffer(true);
    m_currentProcessNodeId = nextNodeId;
    m_currentProcessNodeIdx++;
    m_nextNodeId = findNextNode();
}

void PipeData::addNotifierForNode(cflow_id_t nodeId, cflow_id_t notifierId) { m_nodeNotifiers[nodeId].push_back(notifierId); }

std::vector<cflow_id_t> PipeData::getNotifiersByNodeId(cflow_id_t nodeId)
{
    if (m_nodeNotifiers.count(nodeId) > 0)
    {
        return m_nodeNotifiers[nodeId];
    }
    return {};
}

// private
bool PipeData::checkDependencyValid()
{
    if (m_currentProcessNodeIdx < 0 || m_currentProcessNodeIdx >= (int)m_dependencies.size())
    {
        CFLOW_LOGE("current process node index {0} is error. please check it.", m_currentProcessNodeIdx);
        return false;
    }
    Dependency currentDependency = m_dependencies[m_currentProcessNodeIdx];
    if (currentDependency.curNodeId != m_currentProcessNodeId)
    {
        CFLOW_LOGE("current process node dependency's node [{0}] must equal current process node [{1}].", currentDependency.curNodeId, m_currentProcessNodeId);
        return false;
    }
    return true;
}

void PipeData::addInputForNode(cflow_id_t nodeId, const BufferSpecification& bfs)
{
    if (m_dependenciesNodeInfo.count(nodeId) == 0)
    {
        CFLOW_LOGE("can't find node{0} in data path", nodeId);
    }
    auto nodeInfo = m_dependenciesNodeInfo[nodeId];
    m_buffeManagerFactory->createBufferManager(bfs);
    nodeInfo->input.push_back(bfs);
    CFLOW_LOGD("node{0} add input buffer{1} success!", nodeId, bfs.name);
}

void PipeData::addOutputForNode(cflow_id_t nodeId, const BufferSpecification& bfs)
{
    if (m_dependenciesNodeInfo.count(nodeId) == 0)
    {
        CFLOW_LOGE("can't find node{0} in data path", nodeId, bfs.name);
    }
    auto nodeInfo = m_dependenciesNodeInfo[nodeId];
    m_buffeManagerFactory->createBufferManager(bfs);
    nodeInfo->output.push_back(bfs);
    CFLOW_LOGD("node{0} add output buffer{1} success!", nodeId, bfs.name);
}

bool PipeData::setCurrentNodeIO()
{
    bool ret = true;

    auto currentNodeBufferInfo = m_nodeBufferInfoMap.find(m_currentProcessNodeId);
    auto currentDependency     = m_dependenciesNodeInfo[m_currentProcessNodeId];

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
        CFLOW_LOGD("set data {0} output buffer {1} for current node {2}", ID(), bufInfo->name, m_currentProcessNodeId);
        // not last
        if (m_nextNodeId > 0)
        {
            // input of current node is same as output of next node.
            m_nodeBufferInfoMap[m_nextNodeId].input.push_back(bufInfo);
            CFLOW_LOGD("set data {0} input buffer {1} for next node {2}", ID(), bufInfo->name, m_nextNodeId);
        }
    }

    return ret;
}

void PipeData::releaseCurrentNodeBuffer(bool isInput)
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
                CFLOW_LOGE("error!! can't find buffer manager {0}. can't release buffer", inputBFS.name);
                continue;
            }

            auto it = std::find_if(currentNodeBufferInfo.input.begin(), currentNodeBufferInfo.input.end(), [&inputBFS](PipeDataBufferInfoSP& bufInfo) { return bufInfo->name == inputBFS.name; });
            CFLOW_LOGD("[weiyanyu] currentNodeBufferInfo input size() = {0}", currentNodeBufferInfo.input.size());
            if (it == currentNodeBufferInfo.input.end())
            {
                CFLOW_LOGE("error!! can't find buffer info {0}", inputBFS.name);
                continue;
            }
            bufMgr->pushBuffer(*it);
            CFLOW_LOGD("release {0} input buffer {1} of node {2}", ID(), inputBFS.name, m_currentProcessNodeId);
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
                CFLOW_LOGE("error!! can't find buffer manager {0}. can't release buffer", outputBFS.name);
                continue;
            }

            auto it = std::find_if(currentNodeBufferInfo.output.begin(), currentNodeBufferInfo.output.end(), [&outputBFS](PipeDataBufferInfoSP& bufInfo) { return bufInfo->name == outputBFS.name; });
            if (it == currentNodeBufferInfo.output.end())
            {
                CFLOW_LOGE("error!! can't find buffer info {0}", outputBFS.name);
                continue;
            }
            bufMgr->pushBuffer(*it);
            CFLOW_LOGD("release {0} output buffer {1} of node {2}", ID(), outputBFS.name, m_currentProcessNodeId);
        }
    }
}

void PipeData::dumpDataInfo()
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

}  // namespace pipeline
}  // namespace cflow