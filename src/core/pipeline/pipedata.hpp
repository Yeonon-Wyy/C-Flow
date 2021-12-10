/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 17:45:25
 * @LastEditors: yeonon
 * @LastEditTime: 2021-12-10 23:04:20
 */
#pragma once

#include "../utils/id_generator.hpp"
#include "../utils/str_convertor.hpp"
#include "common_types.hpp"
#include "pipenode_dispatcher.hpp"
#include "../notifier.hpp"

namespace vtf {
namespace pipeline {

using GraphType = std::unordered_map<long, std::vector<long>>;


/**
 * @name: class Request
 * @Descripttion: it is a pipeline data object. 
 *                but this class just a pure virtual class. provider some interface, users must implement these interfaces.
 *                
 * @param {*}
 * @return {*}
 */
class Data {

public:
    Data()
        :m_id(m_idGenerator.generate())
    {}

    virtual ~Data() {
    }


    long ID() { return m_id; };

    /**
     * @name: constructDependency
     * @Descripttion: use pipelines to construct dependency for current PipelineData
     * @param {*}
     * @return {*}
     */    
    virtual bool constructDependency(const std::vector<long>&) = 0;

    /**
     * @name: findNextNodes
     * @Descripttion: find next node list, the list size can only include one nodes or multi nodes, it decide by impl class
     * @param {*}
     * @return {*}
     */    
    // virtual std::vector<long> findNextNodes() = 0;

    /**
     * @name: getCurrentNodes
     * @Descripttion: get current processing node list. the list size can only include one nodes or multi nodes, it decide by impl class
     * @param {*}
     * @return {*}
     */    
    virtual long getCurrentNode() = 0;

    /**
     * @name: getNextNode
     * @Descripttion: get next process node list. the list size can only include one nodes or multi nodes, it decide by impl class
     * @param {*}
     * @return {*}
     */    
    virtual long getNextNode() = 0;


    /**
     * @name: checkDependencyIsReady
     * @Descripttion: check current request state, if is ready will return true, or else will return false
     * @param {*}
     * @return {*}
     */    
    virtual bool checkDependencyIsReady() = 0;

    /**
     * @name: markCurrentNodeReady
     * @Descripttion: mark current node is ready. will effect next node dependency setting
     * @param {*}
     * @return {*}
     */    
    virtual void markCurrentNodeReady() = 0;

    /**
     * @name: scenario
     * @Descripttion: get scenario of this request
     * @param {*}
     * @return {*}
     */    
    virtual PipelineScenario scenario() = 0;

    /**
     * @name: setNotifyStatus
     * @Descripttion: just set notifier status, default is OK
     * @param {NotifyStatus&&} status
     * @return {*}
     */    
    virtual void setNotifyStatus(NotifyStatus&& status) = 0;

    /**
     * @name: getNotifyStatus
     * @Descripttion: return a NotifyStatus
     * @param {*}
     * @return {*}
     */    
    virtual NotifyStatus getNotifyStatus() = 0;

    /**
     * @name: addNotifierForNode
     * @Descripttion: add a notifier for node, when node done, the specified Notifier will be called when node process done
     * @param {long} notifierId
     * @param {long} nodeId
     * @return {*}
     */    
    virtual void addNotifierForNode(long nodeId, long notifierId) = 0;

    /**
     * @name: getNotifiersByNodeId
     * @Descripttion: get notifiers by node id
     * @param {long} nodeId
     * @return {*}
     */    
    virtual std::vector<long> getNotifiersByNodeId(long nodeId) = 0;
private:
    static vtf::utils::IDGenerator m_idGenerator;
    long m_id;
};

vtf::utils::IDGenerator Data::m_idGenerator;

/**
 * @name: class PipeData
 * @Descripttion: it is a sample code for user. just default implementation for Request class.
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
        long curNodeId = -1;
        std::pair<long, DependencyStatus> precursors;
        std::pair<long, DependencyStatus> successors;
    };


    PipeData(PipelineScenario scenario, bool enableDebug = false);

    ~PipeData()
    {
        VTF_LOGD("pipe request {0} destory", ID());
    }

    bool constructDependency(const std::vector<long>& pipeline) override;

    PipelineScenario scenario() override { return m_scenario; }

    long getCurrentNode() override { return m_currentProcessNodeId; };
    long getNextNode() override { return {m_nextNodeId}; };
    
    bool checkDependencyIsReady() override;

    void markCurrentNodeReady() override;

    void setNotifyStatus(NotifyStatus&& status) override { m_notifyStatus = std::move(status); };

    NotifyStatus getNotifyStatus() { return m_notifyStatus; }

    void addNotifierForNode(long nodeId, long notifierId) override;

    std::vector<long> getNotifiersByNodeId(long nodeId) override;
private:
    bool checkDependencyValid();
    long findNextNode();
    bool notifyResult();
private:
    std::vector<Dependency> m_dependencies;
    std::unordered_map<long, std::vector<long>> m_nodeNotifiers;
    PipelineScenario m_scenario;
    NotifyStatus m_notifyStatus;
    long m_currentProcessNodeId;
    int m_currentProcessNodeIdx;
    long m_nextNodeId;
    int m_nextNodeIdx;
    bool m_enableDebug;

};

PipeData::PipeData(PipelineScenario scenario, bool enableDebug)
    :Data(),
        m_scenario(scenario),
        m_notifyStatus(NotifyStatus::OK),
        m_currentProcessNodeId(-1),
        m_currentProcessNodeIdx(-1),
        m_nextNodeId(-1),
        m_nextNodeIdx(-1),
        m_enableDebug(enableDebug)
{
    
}


bool PipeData::constructDependency(const std::vector<long>& pipeline)
{
    m_dependencies.clear();
    for (int i = 0; i < pipeline.size(); i++) {
        long curNodeId = pipeline[i];

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
        VTF_LOGE("dependency size of pipe request can't be less than 2");
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

long PipeData::findNextNode()
{
    if (!checkDependencyValid()) return -1;
    Dependency currentDependency = m_dependencies[m_currentProcessNodeIdx];
    long successorId = currentDependency.successors.first;   
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
    long precursorId = currentDependency.precursors.first;
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
    long nextNodeId = findNextNode();
    m_nextNodeIdx = m_currentProcessNodeIdx + 1;
    //last node
    if (nextNodeId == -1 || m_nextNodeIdx >= m_dependencies.size()) {
        VTF_LOGD("request {0} node [{1}] have done.", ID(), m_currentProcessNodeId);
        m_currentProcessNodeId = -1;
        m_currentProcessNodeIdx++;
        m_nextNodeId = -1;
        m_nextNodeIdx = -1;
        VTF_LOGD("request {0} all node already process done.", ID());
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
    VTF_LOGD("request {0} node [{1}] have done. ", ID(), m_currentProcessNodeId);

    m_currentProcessNodeId = nextNodeId;
    m_currentProcessNodeIdx++;
    m_nextNodeId = findNextNode();
}

void PipeData::addNotifierForNode(long nodeId, long notifierId)
{
    m_nodeNotifiers[nodeId].push_back(notifierId);
}

std::vector<long> PipeData::getNotifiersByNodeId(long nodeId)
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