/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 17:45:25
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-10 22:05:44
 */
#pragma once

#include "../utils.hpp"
#include "common_types.hpp"

namespace vtf {
namespace pipeline {

using GraphType = std::unordered_map<long, std::vector<long>>;

class Request {

public:
    Request()
        :m_id(m_idGenerator.generate())
    {}

    long ID() { return m_id; };
    virtual bool constructDependency(const std::vector<long>&) = 0;
    virtual std::vector<long> findNextNodes() = 0;
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

    bool constructDependency(const std::vector<long>& pipeline) override;

    PipelineScenario scenario() { return m_scenario; }

    std::vector<long> findNextNodes();
private:
    std::vector<Dependency> m_dependencies;
    PipelineScenario m_scenario;
    long m_currentProcessNodeId;
    bool m_enableDebug;
};

PipeRequest::PipeRequest(PipelineScenario scenario, bool enableDebug)
    :Request(),
        m_scenario(scenario),
        m_currentProcessNodeId(-1),
        m_enableDebug(enableDebug)
{
    
}


bool PipeRequest::constructDependency(const std::vector<long>& pipeline)
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

    //dump
    if (m_enableDebug) {
        for (auto& dependency : m_dependencies) {
            VTF_LOGD("node : {0}", dependency.curNodeId);
            VTF_LOGD("pre : {0}", dependency.precursors.first);
            VTF_LOGD("suc : {0}", dependency.successors.first);
            VTF_LOGD("--------------------------------------");  
        }
    }

    return true;
}

//TODO: 需要完成该函数
std::vector<long> PipeRequest::findNextNodes()
{

}

} //namespace pipeline
} //namespace vtf