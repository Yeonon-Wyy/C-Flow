/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 17:45:25
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-06-25 19:38:06
 */
#pragma once

#include "../utils/id_generator.hpp"
#include "../utils/str_convertor.hpp"
#include "type.hpp"
#include "pipenode_dispatcher.hpp"
#include "../data.hpp"
#include "../utils/memory/buffer_manager.hpp"
#include "../utils/memory/buffer_manager_factory.hpp"

using namespace vtf::utils::memory;

namespace vtf {
namespace pipeline {
/**
 * @name: class PipeData
 * @Descripttion: it is a sample code for user. just default implementation for Data class.
 *                users can use it directly, and if the user wants to do some customization, can inherit it and override some interfaces.
 *                WARNING: user can't change this class.
 * @param {*}
 * @return {*}
 */
class PipeData: public Data {
public:
    enum DependencyStatus
    {
        NOREADY = 0,
        READY,
        DONE
    };

    struct DependencyNodeInfo {
        vtf_id_t nodeId;
        DependencyStatus status;
        std::vector<BufferSpecification> input;
        std::vector<BufferSpecification> output;
    };
    using DependencyNodeInfoSP = std::shared_ptr<DependencyNodeInfo>;
    struct Dependency {
        vtf_id_t curNodeId = -1;
        std::pair<vtf_id_t, DependencyNodeInfoSP> precursors;
        std::pair<vtf_id_t, DependencyNodeInfoSP> successors;
    };


    PipeData(PipelineScenario scenario, bool enableDebug = false);

    ~PipeData()
    {
    }

    bool constructDependency(const std::vector<vtf_id_t>& pipeline, BufferManagerFactory<int>& bufferMgrFactory) override;

    virtual bool constructIO(BufferManagerFactory<int>&) override; 

    PipelineScenario scenario() override { return m_scenario; }

    vtf_id_t getCurrentNode() override { return m_currentProcessNodeId; };

    vtf_id_t getNextNode() override { return {m_nextNodeId}; };

    bool checkDependencyIsReady() override;

    void markCurrentNodeReady() override;

    void setNotifyStatus(NotifyStatus&& status) override { m_notifyStatus = std::move(status); };

    NotifyStatus getNotifyStatus() { return m_notifyStatus; }

    void setDataType(DataType&& dataType) { m_dataType = std::move(dataType); }

    DataType getDataType() { return m_dataType; }

    void setPriority(DataPriority&& priority) { m_priority = std::move(priority); }

    DataPriority getPriority() { return m_priority; }

    void addNotifierForNode(vtf_id_t nodeId, vtf_id_t notifierId) override;

    std::vector<vtf_id_t> getNotifiersByNodeId(vtf_id_t nodeId) override;

    void addInputForNode(vtf_id_t nodeId, const BufferSpecification&, BufferManagerFactory<int>& bufferMgrFactory);
    void addOutputForNode(vtf_id_t nodeId, const BufferSpecification&, BufferManagerFactory<int>& bufferMgrFactory);
private:
    bool checkDependencyValid();

    vtf_id_t findNextNode();

    bool notifyResult();
private:
    std::vector<Dependency> m_dependencies;
    std::unordered_map<int, DependencyNodeInfoSP> m_dependenciesNodeInfo;
    std::unordered_map<vtf_id_t, std::vector<vtf_id_t>> m_nodeNotifiers;
    PipelineScenario m_scenario;
    NotifyStatus m_notifyStatus;
    DataType m_dataType;
    DataPriority m_priority;
    vtf_id_t m_currentProcessNodeId;
    int m_currentProcessNodeIdx;
    vtf_id_t m_nextNodeId;
    int m_nextNodeIdx;
    bool m_enableDebug;
};

PipeData::PipeData(PipelineScenario scenario, bool enableDebug)
    :Data(),
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


bool PipeData::constructDependency(const std::vector<vtf_id_t>& pipeline, BufferManagerFactory<int>& bufferMgrFactory)
{
    m_dependencies.clear();

    auto constructDependencyNodeInfo = [&](vtf_id_t nodeId, DependencyStatus status){
        auto nodeInfoSp = std::make_shared<DependencyNodeInfo>();
        nodeInfoSp->nodeId = nodeId;
        nodeInfoSp->status = status;
        return nodeInfoSp;
    };

    auto constructNodeInfo = [&,this]() {
        for (size_t i = 0; i < pipeline.size(); i++) {
            auto nodeInfo = constructDependencyNodeInfo(pipeline[i], DependencyStatus::NOREADY);
            this->m_dependenciesNodeInfo[nodeInfo->nodeId] = nodeInfo;
        }
        //construct a empty node as dummy node
        auto dummyNode = constructDependencyNodeInfo(-1, DependencyStatus::DONE);
        this->m_dependenciesNodeInfo[-1] = dummyNode;
    };
    
    //construct all node info bt pipeline
    constructNodeInfo();

    for (size_t i = 0; i < pipeline.size(); i++) {
        vtf_id_t curNodeId = pipeline[i];

        //check dependency exist
        auto it = std::find_if(m_dependencies.begin(), m_dependencies.end(), [curNodeId](const Dependency& dependency) {
            return dependency.curNodeId == curNodeId;
        });
        if (it != m_dependencies.end()) {
            VTF_LOGE("already exist dependency of node {0}, please check it first.", curNodeId);
            m_dependencies.clear();
        }

        Dependency dependency = Dependency{.curNodeId = curNodeId};
        if (i >= 1) {
            dependency.precursors = {pipeline[i-1], m_dependenciesNodeInfo[pipeline[i-1]]};
        } else {
            dependency.precursors = {-1, m_dependenciesNodeInfo[-1]};
        }
        if (i + 1 < pipeline.size()) {
            dependency.successors = {pipeline[i+1], m_dependenciesNodeInfo[pipeline[i+1]]};
        } else {
            dependency.successors = {-1, m_dependenciesNodeInfo[-1]};
        }
        m_dependencies.push_back(dependency);
    }
    
    if (m_dependencies.empty() || m_dependencies.size() <= 1) {
        VTF_LOGE("dependency size of pipe data can't be less than 2");
        return false;
    }

    //set current process node to first node
    m_currentProcessNodeId = m_dependencies[0].curNodeId;
    m_currentProcessNodeIdx = 0;
    m_nextNodeIdx = m_currentProcessNodeIdx + 1;
    m_nextNodeId = findNextNode();

    //construct input and output
    constructIO(bufferMgrFactory);

    //dump
    if (m_enableDebug) {
        for (auto& dependency : m_dependencies) {
            VTF_LOGD("node : [{0}]", dependency.curNodeId);

            VTF_LOGD("pre : [{0}]", dependency.precursors.first);
            VTF_LOGD("\tinput : ");
            for (auto&& bfs : dependency.precursors.second->input) {
                VTF_LOGD("{0} ", bfs.name);
            }
            VTF_LOGD("\toutput : ");
            for (auto&& bfs : dependency.precursors.second->output) {
                VTF_LOGD("{0} ", bfs.name);
            }
            VTF_LOGD("suc : [{0}]", dependency.successors.first);
            VTF_LOGD("\tinput : ");
            for (auto&& bfs : dependency.successors.second->input) {
                VTF_LOGD("{0} ", bfs.name);
            }
            VTF_LOGD("\toutput : ");
            for (auto&& bfs : dependency.successors.second->output) {
                VTF_LOGD("{0} ", bfs.name);
            }
            VTF_LOGD("--------------------------------------");  
        }
    }

    return true;
}

bool PipeData::constructIO(BufferManagerFactory<int>&)
{
    //noting need to do by default
    return true;
}

vtf_id_t PipeData::findNextNode()
{
    if (!checkDependencyValid()) return -1;
    Dependency currentDependency = m_dependencies[m_currentProcessNodeIdx];
    vtf_id_t successorId = currentDependency.successors.first;   
    DependencyStatus successorStatus = currentDependency.successors.second->status;
    if (successorId != -1 && successorStatus == DependencyStatus::NOREADY) {
        return successorId;
    }
    return -1;
}

bool PipeData::checkDependencyIsReady()
{
    if (!checkDependencyValid()) return false;
    Dependency currentDependency = m_dependencies[m_currentProcessNodeIdx];
    vtf_id_t precursorId = currentDependency.precursors.first;
    DependencyStatus precursorStatus = currentDependency.precursors.second->status;

    if ((precursorId == -1 && precursorStatus == DependencyStatus::DONE)
        || (precursorId != -1 && precursorStatus == DependencyStatus::READY)) 
    {
        VTF_LOGD("precursor node [{0}] is ready", precursorId);
        return true;
    }
    return false;
}


void PipeData::markCurrentNodeReady()
{
    vtf_id_t nextNodeId = findNextNode();
    m_nextNodeIdx = m_currentProcessNodeIdx + 1;
    //last node
    if (nextNodeId == -1 || m_nextNodeIdx >= (int)m_dependencies.size()) {
        VTF_LOGD("data {0} node [{1}] have done.", ID(), m_currentProcessNodeId);
        m_currentProcessNodeId = -1;
        m_currentProcessNodeIdx++;
        m_nextNodeId = -1;
        m_nextNodeIdx = -1;
        VTF_LOGD("data {0} all node already process done.", ID());
        return;
    }

    if (m_dependencies[m_nextNodeIdx].precursors.first != m_dependencies[m_currentProcessNodeIdx].curNodeId) {
        VTF_LOGD("node [{0}] and node [{1}] no connection. please check dependency.", 
            m_dependencies[m_currentProcessNodeIdx].curNodeId, 
            m_dependencies[m_nextNodeIdx].precursors.first
        );
        return;
    }

    //mark next node denpendency's pre is done
    m_dependencies[m_nextNodeIdx].precursors.second->status = DependencyStatus::READY;
    VTF_LOGD("data {0} node [{1}] have done. ", ID(), m_currentProcessNodeId);

    m_currentProcessNodeId = nextNodeId;
    m_currentProcessNodeIdx++;
    m_nextNodeId = findNextNode();
}

void PipeData::addNotifierForNode(vtf_id_t nodeId, vtf_id_t notifierId)
{
    m_nodeNotifiers[nodeId].push_back(notifierId);
}

std::vector<vtf_id_t> PipeData::getNotifiersByNodeId(vtf_id_t nodeId)
{
    if (m_nodeNotifiers.count(nodeId) > 0) {
        return m_nodeNotifiers[nodeId];
    }
    return {};
}

//private
bool PipeData::checkDependencyValid()
{
    if (m_currentProcessNodeIdx < 0 || m_currentProcessNodeIdx >= (int)m_dependencies.size()) {
        VTF_LOGE("current process node index {0} is error. please check it.", m_currentProcessNodeIdx);
        return false;
    }
    Dependency currentDependency = m_dependencies[m_currentProcessNodeIdx];
    if (currentDependency.curNodeId != m_currentProcessNodeId) {
        VTF_LOGE("current process node dependency's node [{0}] must equal current process node [{1}].", 
            currentDependency.curNodeId, 
            m_currentProcessNodeId
        );
        return false;
    }
    return true;
}

void PipeData::addInputForNode(vtf_id_t nodeId, const BufferSpecification& bfs, BufferManagerFactory<int>& bufferMgrFactory)
{
    if (m_dependenciesNodeInfo.count(nodeId) == 0) {
        VTF_LOGE("can't find node{0} in data path", nodeId);
    }
    auto nodeInfo = m_dependenciesNodeInfo[nodeId];
    bufferMgrFactory.createBufferManager(bfs);
    nodeInfo->input.push_back(bfs);
    VTF_LOGD("node{0} add input buffer{1} success!", nodeId, bfs.name);
}

void PipeData::addOutputForNode(vtf_id_t nodeId, const BufferSpecification& bfs, BufferManagerFactory<int>& bufferMgrFactory)
{
    if (m_dependenciesNodeInfo.count(nodeId) == 0) {
        VTF_LOGE("can't find node{0} in data path", nodeId);
    }
    auto nodeInfo = m_dependenciesNodeInfo[nodeId];
    bufferMgrFactory.createBufferManager(bfs);
    nodeInfo->output.push_back(bfs);
    VTF_LOGD("node{0} add output buffer{1} success!");
}

} //namespace pipeline
} //namespace vtf