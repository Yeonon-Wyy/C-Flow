/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-24 16:17:33
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-16 22:53:52
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
    public std::enable_shared_from_this<PipeNode<Item>>,
    public ThreadLoop
{
public:
    using ProcessFunction = std::function<bool(std::shared_ptr<Item>)>;
    using ItemQueue = vtf::BlockingQueue<std::shared_ptr<Item>>;

    PipeNode(int queueSize = defaultPipeNodeQueueSize)
         :DAGNode(m_idGenerator.generate()),
         m_id(getNodeId()),
         m_status(PipeNodeStatus::IDLE),
         m_isStop(false),
         m_processQueue(queueSize)
    {
        m_name = PIPENODE_DEFAULT_NAME_PREFIX + vtf::util::StringConvetor::digit2String(m_id);
        run();
    }

    PipeNode(const std::string& name, int queueSize = defaultPipeNodeQueueSize)
         :DAGNode(m_idGenerator.generate()),
         m_id(getNodeId()),
         m_name(name),
         m_status(PipeNodeStatus::IDLE),
         m_isStop(false),
         m_processQueue(queueSize)
    {
        run();
    }

    ~PipeNode() {}

    bool threadLoop() override;

    void submit(std::shared_ptr<Item>);

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
    void stop() 
    { 
        std::unique_lock<std::mutex> lk(m_mutex);
        m_isStop = true; 
        VTF_LOGD("node [{0}:[1]] stop", m_id, name()); 
    }

    /**
     * @name: restart
     * @Descripttion: restart process
     * @param {*}
     * @return {*}
     */   
    
    void restart() 
    { 
        std::unique_lock<std::mutex> lk(m_mutex); 
        m_isStop = false; 
        m_status = PipeNodeStatus::IDLE; 
    }

    PipeNodeStatus status() { return m_status; }

private:
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
    bool m_isStop;
    std::vector<PipelineScenario> m_pipelineScenarios;
    ProcessFunction m_processFunction;
    ItemQueue m_processQueue;
    std::mutex m_mutex;
};

template<typename Item>
vtf::util::IDGenerator PipeNode<Item>::m_idGenerator;

template<typename Item>
bool PipeNode<Item>::threadLoop()
{
    bool ret = true;
    auto item = m_processQueue.pop();
    ret = process(item);
    return ret;
}

template<typename Item>
void PipeNode<Item>::submit(std::shared_ptr<Item> item)
{
    m_processQueue.push(item);
}

template<typename Item>
bool PipeNode<Item>::process(std::shared_ptr<Item> item)
{
    bool ret = true;
    std::unique_lock<std::mutex> lk(m_mutex);
    if (m_status == PipeNodeStatus::STOP) {
        return false;
    }
    m_status = PipeNodeStatus::PROCESSING;
    ret = m_processFunction(item);
    if (ret) {
        item->markCurrentNodeReady();
    }
    m_status = PipeNodeStatus::IDLE;
    if (m_isStop) {
        m_status = PipeNodeStatus::STOP;
        g_pipeNodeStopCV.notify_one();
    }
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

} //namespace pipeline
} //namespace vtf