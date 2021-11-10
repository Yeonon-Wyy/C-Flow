/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 18:48:53
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-10 21:53:13
 */

#pragma once

#include "../dag.hpp"
#include "pipeNode.hpp"
#include "pipenodeDispatcher.hpp"
#include "../log.hpp"
#include <memory>
#include <mutex>
namespace vtf {
namespace pipeline {



template<typename Item>
class PipeLine {
public:
    PipeLine()
        :m_dag()
    {}

    ~PipeLine()
    {}


    using ProcessFunction = std::function<bool(std::shared_ptr<Item>)>;
    using GraphType = std::unordered_map<long, std::vector<long>>;

    //TODO: 可以作成可变参数，并且返回Tuple
    std::shared_ptr<PipeNode<Item>> addPipeNode(ProcessFunction&& pf);

    //temp:
    bool reorganize();
    std::vector<long> getPipelineWithScenario(PipelineScenario scenario);

    //submit item
    bool submit(std::shared_ptr<Item> item);
    
private:
    DAG m_dag;
    PipeNodeDispatcher<Item> m_pipeNodeDispatcher;
    std::unordered_map<long, std::shared_ptr<PipeNode<Item>>> m_pipeNodeMaps;
    std::vector<std::vector<long>> m_pipelines;
    std::unordered_map<PipelineScenario, std::vector<long>> m_scenario2PipelineMaps;

    bool m_pipelineModified = false;
    std::mutex m_mutex;
};

template<typename Item>
std::shared_ptr<PipeNode<Item>> PipeLine<Item>::addPipeNode(ProcessFunction&& pf)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    std::shared_ptr<PipeNode<Item>> node = std::make_shared<PipeNode<Item>>();
    node->setProcessFunction(std::forward<ProcessFunction>(pf));
    m_pipeNodeMaps[node->getNodeId()] = node;
    m_dag.addNode(node);
    m_pipeNodeDispatcher.addPipeNode(node);
    m_pipelineModified = true;
    return node;
}

template<typename Item>
bool PipeLine<Item>::reorganize()
{
    std::unique_lock<std::mutex> lk(m_mutex);
    auto pipelines = m_dag.multiTopologicalSort();
    for (int scenario = PipelineScenario::SCENARIO_START + 1; scenario < PipelineScenario::SCENARIO_END; scenario++) {
        
        for (auto pipelineIter = pipelines.begin(); pipelineIter != pipelines.end(); pipelineIter++) {
            auto pipeline = *pipelineIter;
            bool findFlag = true;
            for (long &nodeID : pipeline) {
                if (m_pipeNodeMaps.count(nodeID) == 0) {
                    VTF_LOGE("can't find node {0} in m_pipeNodeMaps, please check it.", nodeID);
                    m_scenario2PipelineMaps.clear();
                    return false;
                }
                if (!m_pipeNodeMaps[nodeID]->hasScenario((PipelineScenario)scenario)) {
                    findFlag = false;
                    break;
                }
            }
            if (findFlag) {
                if (m_scenario2PipelineMaps.count((PipelineScenario)scenario)) {
                    VTF_LOGE("There can only be one pipeline in a scenario {0}, please check Graph.", scenario);
                    m_scenario2PipelineMaps.clear();
                    return false;
                }
                m_scenario2PipelineMaps[(PipelineScenario)scenario] = pipeline;
            } 
        }
    }

    //dump
    for (auto&[scenario, pipeline] : m_scenario2PipelineMaps) {
        std::stringstream ss;
        ss << scenario << ": [";
        for (long& nodeId : pipeline) {
            ss << nodeId << " ";
        }
        ss << "]";
        VTF_LOGD(ss.str());
    }

    return true;
}

template<typename Item>
std::vector<long> PipeLine<Item>::getPipelineWithScenario(PipelineScenario scenario)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    std::vector<long> pipeline;
    if (m_scenario2PipelineMaps.count(scenario) == 0) {
        VTF_LOGE("can't find any pipelie about scenario {0}", scenario);
    } else {
        pipeline = m_scenario2PipelineMaps[scenario];
    }

    return pipeline;
}

template<typename Item>
bool PipeLine<Item>::submit(std::shared_ptr<Item> item)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    m_pipeNodeDispatcher.queueInDispacther(item);
    VTF_LOGD("submit a item {0}", item->ID());
}

} //namespace pipeline
} //namespace vtf

