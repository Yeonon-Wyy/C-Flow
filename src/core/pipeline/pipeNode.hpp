/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-24 16:17:33
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-10 20:54:04
 */
#pragma once
#include "../dag.hpp"
#include "../threadLoop.hpp"
#include "../log.hpp"
#include "../utils.hpp"
#include "common_types.hpp"

#include <mutex>
#include <memory>
#include <condition_variable>
#include <unordered_map>

namespace vtf {
namespace pipeline {

#define PIPENODE_DEFAULT_NAME_PREFIX "pipeNode_"



template<typename Item>
class PipeNode : 
    public vtf::DAGNode,
    public std::enable_shared_from_this<PipeNode<Item>>
{
public:
    using ProcessFunction = std::function<bool(std::shared_ptr<Item>)>;

    PipeNode()
         :DAGNode(m_idGenerator.generate()),
         m_id(getNodeId())
    {
        m_name = PIPENODE_DEFAULT_NAME_PREFIX + vtf::util::StringConvetor::digit2String(m_id);
    }

    PipeNode(std::string&& name)
         :DAGNode(m_idGenerator.generate()),
         m_id(getNodeId()),
         m_name(std::move(name))
    {}

    ~PipeNode() {}

    bool process();

    void setProcessFunction(ProcessFunction&& pf) { m_processFunction = std::move(pf); }
    const std::string name() const { return m_name; }

    PipeNode* addcenarios(PipelineScenario scenario);
    bool hasScenario(PipelineScenario scenario);

private:
    static vtf::util::IDGenerator m_idGenerator;
    long m_id;
    std::string m_name;
    std::vector<PipelineScenario> m_pipelineScenarios;

    ProcessFunction m_processFunction;
    std::mutex m_mutex;
};

// [1,3,2,5,6]
// [1,3,2,5,7]

//[SAT: [1,3,2,5,6]]
//[BOKEH: [1,3,2,5,7]]

template<typename Item>
vtf::util::IDGenerator PipeNode<Item>::m_idGenerator;

template<typename Item>
bool PipeNode<Item>::process()
{
    bool ret = true;
    std::unique_lock<std::mutex> lk(m_mutex);
    ret = m_processFunction();
    return ret;
}

template<typename Item>
PipeNode<Item>* PipeNode<Item>::addcenarios(PipelineScenario scenario)
{
    m_pipelineScenarios.push_back(scenario);
    return this;
}

template<typename Item>
bool PipeNode<Item>::hasScenario(PipelineScenario scenario)
{
    return std::find(m_pipelineScenarios.begin(), m_pipelineScenarios.end(), scenario) != m_pipelineScenarios.end();
}

} //namespace pipeline
} //namespace vtf