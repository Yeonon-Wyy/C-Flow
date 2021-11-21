/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 18:48:53
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-21 21:46:35
 */

#pragma once

#include "../dag.hpp"
#include "pipeNode.hpp"
#include "pipeNodeDispatcher.hpp"
#include "../notifier.hpp"
#include "../log.hpp"
#include <memory>
#include <mutex>
namespace vtf {
namespace pipeline {



template<typename Item>
class PipeLine {
public:
    PipeLine()
        :m_dag(),
         m_pipeNodeDispatcher(std::make_shared<PipeNodeDispatcher<Item>>())
    {}

    ~PipeLine()
    {

    }


    using ProcessFunction = std::function<bool(std::shared_ptr<Item>)>;
    using GraphType = std::unordered_map<long, std::vector<long>>;

    /**
     * @name: addPipeNode
     * @Descripttion: add a pipe node to pipeline. pipeline will generate a new node object for user.
     * @param {ProcessFunction&&} pf is node process function.
     * @return {*}
     */    
    std::shared_ptr<PipeNode<Item>> addPipeNode(const std::string& name, ProcessFunction&& pf);

    /**
     * @name: addPipeNode
     * @Descripttion: add a pipe node, user will construct node object self.
     * @param {shared_ptr<PipeNode<Item>>} node
     * @return {*}
     */    
    bool addPipeNode(std::shared_ptr<PipeNode<Item>> node);

    void addNotifier(std::shared_ptr<Notifier<Item>> notifier);
    void addNotifier(const std::string& name, int notifierQueueSize, std::function<bool(std::shared_ptr<Item>)>&& pf);

    /**
     * @name: constructPipelinesByScenario
     * @Descripttion: construct pipelines by given scenario
     * @param {*}
     * @return {*}
     */    
    bool constructPipelinesByScenario();


    /**
     * @name: submit a item to pipeline
     * @Descripttion: 
     * @param {shared_ptr<Item>} item
     * @return {*}
     */    
    bool submit(std::shared_ptr<Item> item);

    void start();
    void stop();

private:

    /**
     * @name: getPipelineWithScenario
     * @Descripttion: get a pipeline by given scenario
     * @param {PipelineScenario} scenario
     * @return {*}
     */    
    std::vector<long> getPipelineWithScenario(PipelineScenario scenario);

    /**
     * @name: dumpPipelines
     * @Descripttion: just dump pipelines
     * @param {*}
     * @return {*}
     */    
    void dumpPipelines();

private:
    /**
     * @name: checkValid
     * @Descripttion: check pipeline is valid
     * @param {*}
     * @return {*}
     */
    bool checkValid();

private:
    DAG m_dag;
    std::shared_ptr<PipeNodeDispatcher<Item>> m_pipeNodeDispatcher;
    std::unordered_map<long, std::shared_ptr<PipeNode<Item>>> m_pipeNodeMaps;
    std::vector<std::vector<long>> m_pipelines;
    std::unordered_map<PipelineScenario, std::vector<long>> m_scenario2PipelineMaps;
    std::unordered_set<PipelineScenario> m_pipelineScenarioSet;
    std::vector<std::shared_ptr<Notifier<Item>>> m_notifiers;

    std::atomic_bool m_isStop = false;
    bool m_pipelineModified = false;
    long m_finalSubmitedItemID = -1;

    std::mutex m_mutex;
};

template<typename Item>
bool PipeLine<Item>::checkValid()
{
    if (m_isStop) {
        VTF_LOGD("pipeline already stoped, can't submit any item, please construct another pipeline obejct!!!!");
        return false;
    }
    return true;
}

template<typename Item>
std::shared_ptr<PipeNode<Item>> PipeLine<Item>::addPipeNode(const std::string& name, ProcessFunction&& pf)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (!checkValid()) return nullptr;
    std::shared_ptr<PipeNode<Item>> node = std::make_shared<PipeNode<Item>>(name);
    node->setProcessFunction(std::forward<ProcessFunction>(pf));
    m_pipeNodeMaps[node->getNodeId()] = node;
    m_dag.addNode(node);
    m_pipeNodeDispatcher->addPipeNode(node);
    m_pipelineModified = true;
    return node;
}
template<typename Item>
bool PipeLine<Item>::addPipeNode(std::shared_ptr<PipeNode<Item>> node)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (!checkValid()) return false;
    if (m_pipeNodeMaps.count(node->getNodeId()) != 0) {
        VTF_LOGE("node [{0}:{1}] already be added to pipeline. don't need add again.", node->getNodeId(), node->name());
        return false;
    }

    m_pipeNodeMaps[node->getNodeId()] = node;
    m_dag.addNode(node);
    m_pipeNodeDispatcher->addPipeNode(node);
    m_pipelineModified = true;
    return true;
}

template<typename Item>
void PipeLine<Item>::addNotifier(std::shared_ptr<Notifier<Item>> notifier)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (!checkValid()) return;
    m_notifiers.push_back(notifier);
}

template<typename Item>
void PipeLine<Item>::addNotifier(const std::string& name, int notifierQueueSize, std::function<bool(std::shared_ptr<Item>)>&& pf)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (!checkValid()) return;
    auto notifier = std::make_shared<Notifier<Item>>(name, notifierQueueSize, std::forward<std::function<bool(std::shared_ptr<Item>)>>(pf));
    m_notifiers.push_back(notifier);
}

template<typename Item>
bool PipeLine<Item>::constructPipelinesByScenario()
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (!checkValid()) return false;
    if (!m_pipelineModified) return true;
    VTF_LOGD("m_notifiers.size {0}", m_notifiers.size());
    for (auto notifier : m_notifiers) {
        m_pipeNodeDispatcher->addResultNotifier(notifier);
    }

    //get all scenario
    for (auto&[nodeId, node] : m_pipeNodeMaps) {
        for (PipelineScenario scenario : node->getScenarios()) {
            m_pipelineScenarioSet.insert(scenario);
        }
    }
    auto pipelines = m_dag.multiTopologicalSort();
    for (auto it = m_pipelineScenarioSet.begin(); it != m_pipelineScenarioSet.end(); it++) {
        PipelineScenario scenario = *it;
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

    m_pipelineModified = false;
    dumpPipelines();
    return true;
}

template<typename Item>
std::vector<long> PipeLine<Item>::getPipelineWithScenario(PipelineScenario scenario)
{
    if (!checkValid()) return {};
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
    if (!checkValid()) return false;
    item->constructDependency(getPipelineWithScenario(item->scenario()), m_pipeNodeDispatcher);
    m_pipeNodeDispatcher->queueInDispacther(item);
    VTF_LOGD("submit a item {0}", item->ID());
    m_finalSubmitedItemID = item->ID();
    return true;
}

template<typename Item>
void PipeLine<Item>::start()
{
    if (!checkValid()) return;
    constructPipelinesByScenario();
}

template<typename Item>
void PipeLine<Item>::stop()
{
    std::unique_lock<std::mutex> lk(m_mutex);
    m_isStop = true;
    m_pipeNodeDispatcher->stop();
    for (auto&[nodeId, node] : m_pipeNodeMaps) {
        node->stop();
    }
    for (auto notifier : m_notifiers) {
        notifier->stop();
    }

    //clear info
    m_dag.clear();
    m_pipeNodeMaps.clear();
    m_pipelineModified = true;
}

//private
template<typename Item>
void PipeLine<Item>::dumpPipelines()
{
    if (!checkValid()) return;
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
}

} //namespace pipeline
} //namespace vtf

