/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-24 16:17:33
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-27 19:58:18
 */
#pragma once
#include "../dag.hpp"
#include "../threadLoop.hpp"
#include "../log.hpp"
#include "../utils.hpp"
#include "common_types.hpp"

#include <memory>
#include <unordered_map>

namespace vtf {
namespace pipeline {

#define PIPENODE_DEFAULT_NAME_PREFIX "pipeNode_"
constexpr int defaultPipeNodeQueueSize = 32;

enum PipeNodeStatus {
    PROCESSING,
    IDLE,
    STOP,
};

/**
 * @name: class PipeNode
 * @Descripttion: pipeNode as business processing unit, user only need provide a processCallback, our framework will dispatch some data and call it.
 * @param {*}
 * @return {*}
 */
template<typename Item>
class PipeNodeDispatcher;


template<typename Item>
class PipeNode : 
    public vtf::DAGNode,
    public std::enable_shared_from_this<PipeNode<Item>>
{
public:
    using ProcessCallback = std::function<bool(std::shared_ptr<Item>)>;

    struct PipeNodeCreateInfo {
        PipeNodeId id;
        std::string name;
        std::initializer_list<PipelineScenario> pipelineScenarios;
        ProcessCallback processCallback;
    };

    class PipeNodeBuilder {
    public:
        PipeNodeBuilder()
             :id(-1)
        {}
        PipeNodeBuilder* setID(PipeNodeId&& id);
        PipeNodeBuilder* setName(const std::string& name);
        PipeNodeBuilder* setProcessCallback(ProcessCallback&& cb);
        PipeNodeBuilder* addScenarios(PipelineScenario&& scenario);
        PipeNodeBuilder* addScenarios(std::initializer_list<PipelineScenario> scenarios);

        std::shared_ptr<PipeNode<Item>> build(std::shared_ptr<PipeNodeDispatcher<Item>>);
    private:
        PipeNodeId id;
        std::string name;
        std::vector<PipelineScenario> pipelineScenarios;
        ProcessCallback processCallback;
    };

public:
    PipeNode(PipeNodeId id)
         :DAGNode(id),
         m_id(id),
         m_status(PipeNodeStatus::IDLE)
    {
        m_name = PIPENODE_DEFAULT_NAME_PREFIX + vtf::util::StringConvetor::digit2String(m_id);
    }

    ~PipeNode() {
        VTF_LOGD("node {0} destory", m_name);
    }

    static PipeNodeBuilder builder() { return PipeNodeBuilder(); }

    /**
     * @name: setProcessCallback
     * @Descripttion: setter for user define process function
     * @param {ProcessCallback&&} pf is user define process function
     * @return {*}
     */    
    void setProcessCallback(ProcessCallback&& pf) { m_processCallback = std::move(pf); }

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
    PipeNodeId m_id;
    std::string m_name;
    PipeNodeStatus m_status;
    std::vector<PipelineScenario> m_pipelineScenarios;
    ProcessCallback m_processCallback;
    std::weak_ptr<PipeNodeDispatcher<Item>> m_pipeNodeDispatcher;
};

/*
*
* Implementation of class PipeNode
*/
template<typename Item>
bool PipeNode<Item>::process(std::shared_ptr<Item> item)
{
    bool ret = true;
    m_status = PipeNodeStatus::PROCESSING;
    ret = m_processCallback(item);
    if (ret) {
        item->markCurrentNodeReady();
    }
    auto pipeNodeDispatcherSp = m_pipeNodeDispatcher.lock();
    if (pipeNodeDispatcherSp) {
        pipeNodeDispatcherSp->queueInDispacther(item);
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


/*
*
* Implementation of class PipeNode::PipeNodeBuilder
*/
template<typename Item>
typename PipeNode<Item>::PipeNodeBuilder* PipeNode<Item>::PipeNodeBuilder::setID(PipeNodeId&& id)
{
    this->id = std::move(id);
    return this;
}

template<typename Item>
typename PipeNode<Item>::PipeNodeBuilder* PipeNode<Item>::PipeNodeBuilder::setName(const std::string& name)
{
    this->name = name;
    return this;
}

template<typename Item>
typename PipeNode<Item>::PipeNodeBuilder* PipeNode<Item>::PipeNodeBuilder::setProcessCallback(ProcessCallback&& cb)
{
    this->processCallback = std::move(cb);
    return this;
}

template<typename Item>
typename PipeNode<Item>::PipeNodeBuilder* PipeNode<Item>::PipeNodeBuilder::addScenarios(PipelineScenario&& scenario)
{
    this->pipelineScenarios.push_back(std::move(scenario));
    return this;
}

template<typename Item>
typename PipeNode<Item>::PipeNodeBuilder* PipeNode<Item>::PipeNodeBuilder::addScenarios(std::initializer_list<PipelineScenario> scenarios)
{
    this->pipelineScenarios.insert(this->pipelineScenarios.end(), scenarios.begin(), scenarios.end());
    return this;
}

template<typename Item>
std::shared_ptr<PipeNode<Item>> PipeNode<Item>::PipeNodeBuilder::build(std::shared_ptr<PipeNodeDispatcher<Item>> dispatcher)
{
    if (id < 0) {
        VTF_LOGE("id must set greater than 0");
        return nullptr;
    }

    if (name == "") {
        name = PIPENODE_DEFAULT_NAME_PREFIX + vtf::util::StringConvetor::digit2String(id);
    }

    std::shared_ptr<PipeNode<Item>> pipeNode = std::make_shared<PipeNode<Item>>(id);
    pipeNode->m_name = name;
    pipeNode->m_processCallback = processCallback;
    pipeNode->m_pipelineScenarios = pipelineScenarios;
    pipeNode->m_pipeNodeDispatcher = dispatcher;
    return pipeNode;
}

} //namespace pipeline
} //namespace vtf