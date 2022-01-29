/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 17:45:25
 * @LastEditors: yeonon
 * @LastEditTime: 2022-01-29 14:44:33
 */
#pragma once

#include "../utils/id_generator.hpp"
#include "../utils/str_convertor.hpp"
#include "type.hpp"
#include "pipenode_dispatcher.hpp"
#include "../data.hpp"

namespace vtf {
namespace pipeline {

using GraphType = std::unordered_map<vtf_id_t, std::vector<vtf_id_t>>;

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

    struct Dependency {
        vtf_id_t curNodeId = -1;
        std::pair<vtf_id_t, DependencyStatus> precursors;
        std::pair<vtf_id_t, DependencyStatus> successors;
    };


    PipeData(PipelineScenario scenario, bool enableDebug = false);

    ~PipeData()
    {
    }

    bool constructDependency(const std::vector<vtf_id_t>& pipeline) override;

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
private:
    bool checkDependencyValid();
    vtf_id_t findNextNode();
    bool notifyResult();
private:
    std::vector<Dependency> m_dependencies;
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


bool PipeData::constructDependency(const std::vector<vtf_id_t>& pipeline)
{
    m_dependencies.clear();
    for (int i = 0; i < pipeline.size(); i++) {
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
        if (i - 1 >= 0) {
            dependency.precursors = std::make_pair<>(pipeline[i-1], DependencyStatus::NOREADY);
        } else {
            dependency.precursors = std::make_pair<>(-1, DependencyStatus::DONE);
        }
        if (i + 1 < pipeline.size()) {
            dependency.successors = std::make_pair<>(pipeline[i+1], DependencyStatus::NOREADY);
        } else {
            dependency.successors = std::make_pair<>(-1, DependencyStatus::DONE);
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


    //dump
    if (m_enableDebug) {
        for (auto& dependency : m_dependencies) {
            VTF_LOGD("node : [{0}]", dependency.curNodeId);
            VTF_LOGD("pre : [{0}]", dependency.precursors.first);
            VTF_LOGD("suc : [{0}]", dependency.successors.first);
            VTF_LOGD("--------------------------------------");  
        }
    }

    return true;
}

vtf_id_t PipeData::findNextNode()
{
    if (!checkDependencyValid()) return -1;
    Dependency currentDependency = m_dependencies[m_currentProcessNodeIdx];
    vtf_id_t successorId = currentDependency.successors.first;   
    DependencyStatus successorStatus = currentDependency.successors.second;
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
    DependencyStatus precursorStatus = currentDependency.precursors.second;

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
    if (nextNodeId == -1 || m_nextNodeIdx >= m_dependencies.size()) {
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
    m_dependencies[m_nextNodeIdx].precursors.second = DependencyStatus::READY;
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
    if (m_currentProcessNodeIdx < 0 || m_currentProcessNodeIdx >= m_dependencies.size()) {
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
} //namespace pipeline
} //namespace vtf