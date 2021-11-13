/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 17:45:25
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-13 16:24:53
 */
#pragma once

#include "../utils.hpp"
#include "common_types.hpp"
#include "pipeNodeDispatcher.hpp"

namespace vtf {
namespace pipeline {

using GraphType = std::unordered_map<long, std::vector<long>>;

class Request : public std::enable_shared_from_this<Request>{

public:
    Request()
        :m_id(m_idGenerator.generate())
    {}

    long ID() { return m_id; };

    /**
     * @name: constructDependency
     * @Descripttion: use pipelines to construct dependency for current request
     * @param {*}
     * @return {*}
     */    
    virtual bool constructDependency(const std::vector<long>&, std::shared_ptr<PipeNodeDispatcher<Request>>) = 0;

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

    virtual PipelineScenario scenario() = 0;
private:
    static vtf::util::IDGenerator m_idGenerator;
    long m_id;
};

vtf::util::IDGenerator Request::m_idGenerator;

class PipeRequest : public Request {
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


    PipeRequest(PipelineScenario scenario, bool enableDebug = false);

    ~PipeRequest()
    {
    }

    bool constructDependency(const std::vector<long>& pipeline, std::shared_ptr<PipeNodeDispatcher<Request>> dispatcher) override;

    PipelineScenario scenario() override { return m_scenario; }

    long getCurrentNode() override { return m_currentProcessNodeId; };
    long getNextNode() override { return {m_nextNodeId}; };
    
    bool checkDependencyIsReady() override;

    void markCurrentNodeReady() override;
private:
    bool checkDependencyValid();
    long findNextNode();
private:
    std::vector<Dependency> m_dependencies;
    std::shared_ptr<PipeNodeDispatcher<Request>> m_pipeNodeDIspatcher;
    PipelineScenario m_scenario;
    long m_currentProcessNodeId;
    int m_currentProcessNodeIdx;
    long m_nextNodeId;
    int m_nextNodeIdx;
    bool m_enableDebug;

};

PipeRequest::PipeRequest(PipelineScenario scenario, bool enableDebug)
    :Request(),
        m_scenario(scenario),
        m_currentProcessNodeId(-1),
        m_currentProcessNodeIdx(-1),
        m_nextNodeId(-1),
        m_nextNodeIdx(-1),
        m_enableDebug(enableDebug)
{
    
}


bool PipeRequest::constructDependency(const std::vector<long>& pipeline, std::shared_ptr<PipeNodeDispatcher<Request>> dispatcher)
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

    m_pipeNodeDIspatcher = dispatcher;

    //dump
    if (m_enableDebug) {
        for (auto& dependency : m_dependencies) {
            VTF_LOGD("node : [{0}:{1}]", dependency.curNodeId, m_pipeNodeDIspatcher->getNodeNameByNodeId(dependency.curNodeId));
            VTF_LOGD("pre : [{0}:{1}]", dependency.precursors.first, m_pipeNodeDIspatcher->getNodeNameByNodeId(dependency.precursors.first));
            VTF_LOGD("suc : [{0}:{1}]", dependency.successors.first), m_pipeNodeDIspatcher->getNodeNameByNodeId(dependency.successors.first);
            VTF_LOGD("--------------------------------------");  
        }
    }

    return true;
}

//TODO: 需要完成该函数
long PipeRequest::findNextNode()
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

bool PipeRequest::checkDependencyIsReady()
{
    if (!checkDependencyValid()) return false;
    Dependency currentDependency = m_dependencies[m_currentProcessNodeIdx];
    long precursorId = currentDependency.precursors.first;
    DependencyStatus precursorStatus = currentDependency.precursors.second;

    if ((precursorId == -1 && precursorStatus == DependencyStatus::DONE)
        || (precursorId != -1 && precursorStatus == DependencyStatus::READY)) 
    {
        VTF_LOGD("precursor node [{0}:{1}] is ready", precursorId, m_pipeNodeDIspatcher->getNodeNameByNodeId(precursorId));
        return true;
    }
    return false;
}

void PipeRequest::markCurrentNodeReady()
{
    long nextNodeId = findNextNode();
    m_nextNodeIdx = m_currentProcessNodeIdx + 1;
    //last node
    if (nextNodeId == -1 || m_nextNodeIdx >= m_dependencies.size()) {
        VTF_LOGD("request {0} node [{1}:{2}] have done.", ID(), m_currentProcessNodeId, m_pipeNodeDIspatcher->getNodeNameByNodeId(m_currentProcessNodeId));
        m_currentProcessNodeId = -1;
        m_currentProcessNodeIdx++;
        m_nextNodeId = -1;
        m_nextNodeIdx = -1;
        VTF_LOGD("request {0} all node already process done.", ID());
        return;
    }

    if (m_dependencies[m_nextNodeIdx].precursors.first != m_dependencies[m_currentProcessNodeIdx].curNodeId) {
        VTF_LOGD("node [{0}:{1}] and node [{2}:{3}] no connection. please check dependency.", 
            m_dependencies[m_currentProcessNodeIdx].curNodeId, 
            m_pipeNodeDIspatcher->getNodeNameByNodeId(m_dependencies[m_currentProcessNodeIdx].curNodeId),
            m_dependencies[m_nextNodeIdx].precursors.first,
            m_pipeNodeDIspatcher->getNodeNameByNodeId(m_dependencies[m_nextNodeIdx].precursors.first)
        );
        return;
    }

    //mark next node denpendency's pre is done
    m_dependencies[m_nextNodeIdx].precursors.second = DependencyStatus::READY;
    VTF_LOGD("request {0} node [{1}:{2}] have done. ", ID(), m_currentProcessNodeId, m_pipeNodeDIspatcher->getNodeNameByNodeId(m_currentProcessNodeId));

    m_currentProcessNodeId = nextNodeId;
    m_currentProcessNodeIdx++;
    m_nextNodeId = findNextNode();
    //queue in dispatcher
    m_pipeNodeDIspatcher->queueInDispacther(shared_from_this());
}

//private
bool PipeRequest::checkDependencyValid()
{
    if (m_currentProcessNodeIdx < 0 || m_currentProcessNodeIdx >= m_dependencies.size()) {
        VTF_LOGE("current process node index {0} is error. please check it.", m_currentProcessNodeIdx);
        return false;
    }
    Dependency currentDependency = m_dependencies[m_currentProcessNodeIdx];
    if (currentDependency.curNodeId != m_currentProcessNodeId) {
        VTF_LOGE("current process node dependency's node [{0}:{1}] must equal current process node [{2}:{3}].", 
            currentDependency.curNodeId, 
            m_pipeNodeDIspatcher->getNodeNameByNodeId(currentDependency.curNodeId),
            m_currentProcessNodeId,
            m_pipeNodeDIspatcher->getNodeNameByNodeId(m_currentProcessNodeId)
        );
        return false;
    }
    return true;
}

} //namespace pipeline
} //namespace vtf