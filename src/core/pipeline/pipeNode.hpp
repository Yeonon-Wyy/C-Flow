/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-24 16:17:33
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-21 21:51:04
 */
#pragma once
#include "../dag.hpp"
#include "../threadLoop.hpp"
#include "../log.hpp"
#include "../utils.hpp"
#include "../blocking_queue.hpp"
#include "common_types.hpp"

#include <mutex>
#include <memory>
#include <condition_variable>
#include <unordered_map>

namespace vtf {
namespace pipeline {

#define PIPENODE_DEFAULT_NAME_PREFIX "pipeNode_"
constexpr int defaultPipeNodeQueueSize = 32;

template<typename Item>
class PipeNode : 
    public vtf::DAGNode,
    public std::enable_shared_from_this<PipeNode<Item>>
{
public:
    using ProcessFunction = std::function<bool(std::shared_ptr<Item>)>;
    using ItemQueue = vtf::BlockingQueue<std::shared_ptr<Item>>;

    PipeNode(int queueSize = defaultPipeNodeQueueSize)
         :DAGNode(m_idGenerator.generate()),
         m_id(getNodeId()),
         m_status(PipeNodeStatus::IDLE)
    {
        m_name = PIPENODE_DEFAULT_NAME_PREFIX + vtf::util::StringConvetor::digit2String(m_id);
    }

    PipeNode(const std::string& name, int queueSize = defaultPipeNodeQueueSize)
         :DAGNode(m_idGenerator.generate()),
         m_id(getNodeId()),
         m_name(name),
         m_status(PipeNodeStatus::IDLE)
    {
    }

    ~PipeNode() {
        VTF_LOGD("node {0} destory", m_name);
    }

    /**
     * @name: setProcessFunction
     * @Descripttion: setter for user define process function
     * @param {ProcessFunction&&} pf is user define process function
     * @return {*}
     */    
    void setProcessFunction(ProcessFunction&& pf) { m_processFunction = std::move(pf); }

    /**
     * @name: name
     * @Descripttion: return a node name
     * @param {*}
     * @return {*}
     */    
    const std::string name() const { return m_name; }

    /**
     * @name: addcenarios
     * @Descripttion: add a scenario for pipeNode
     * @param {PipelineScenario} scenario
     * @return {*}
     */    
    PipeNode* addScenarios(PipelineScenario scenario);

    /**
     * @name: addcenarios
     * @Descripttion: add a scenario for pipeNode by initializer List
     * @param {PipelineScenario} scenarios
     * @return {*}
     */    
    PipeNode* addScenarios(std::initializer_list<PipelineScenario> scenarios);

    /**
     * @name: hasScenario
     * @Descripttion: check node is include someone  scenario
     * @param {PipelineScenario} scenario
     * @return {*}
     */    
    bool hasScenario(PipelineScenario scenario);


    std::vector<PipelineScenario> getScenarios() { return m_pipelineScenarios; }
    /**
     * @name: stop
     * @Descripttion: stop process
     * @param {*}
     * @return {*}
     */    
    void stop();

    /**
     * @name: status
     * @Descripttion: return pipeNode status
     * @param {*}
     * @return {*}
     */    
    PipeNodeStatus status() { return m_status; }

    /**
     * @name: process 
     * @Descripttion: execute user define process function, and after complete, mark done
     * @param {*}
     * @return {*}
     */    
    bool process(std::shared_ptr<Item>);
private:
    static vtf::util::IDGenerator m_idGenerator;
    long m_id;
    std::string m_name;
    PipeNodeStatus m_status;
    std::vector<PipelineScenario> m_pipelineScenarios;
    ProcessFunction m_processFunction;
    std::mutex m_mutex;
};

template<typename Item>
vtf::util::IDGenerator PipeNode<Item>::m_idGenerator;


template<typename Item>
bool PipeNode<Item>::process(std::shared_ptr<Item> item)
{
    bool ret = true;
    m_status = PipeNodeStatus::PROCESSING;
    ret = m_processFunction(item);
    if (ret) {
        item->markCurrentNodeReady();
    }
    m_status = PipeNodeStatus::IDLE;
    return ret;
}

template<typename Item>
PipeNode<Item>* PipeNode<Item>::addScenarios(PipelineScenario scenario)
{
    m_pipelineScenarios.push_back(scenario);
    return this;
}
template<typename Item>
PipeNode<Item>* PipeNode<Item>::addScenarios(std::initializer_list<PipelineScenario> scenarios)
{
    m_pipelineScenarios.insert(m_pipelineScenarios.end(), scenarios.begin(), scenarios.end());
    return this;
}

template<typename Item>
bool PipeNode<Item>::hasScenario(PipelineScenario scenario)
{
    return std::find(m_pipelineScenarios.begin(), m_pipelineScenarios.end(), scenario) != m_pipelineScenarios.end();
}

template<typename Item>
void PipeNode<Item>::stop() 
{ 
    VTF_LOGD("PipeNode [{0}] stop start", m_name);
    VTF_LOGD("PipeNode [{0}] stop end", m_name);
    
}

} //namespace pipeline
} //namespace vtf