/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 18:48:53
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-26 21:14:16
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

    /**
     * @name: addNotifier
     * @Descripttion: add a exist notifier object
     * @param {shared_ptr<Notifier<Item>>} notifier
     * @return {*}
     */    
    void addNotifier(std::shared_ptr<Notifier<Item>> notifier);

    /**
     * @name: addNotifier
     * @Descripttion: add a notifier by create infos, will return a notifier object
     * @param {typename} Notifier
     * @return {*}
     */    
    void addNotifier(typename Notifier<Item>::NotifierCreateInfo createInfo);

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

    /**
     * @name: start
     * @Descripttion: start pipeline, must call it
     * @param {*}
     * @return {*}
     */    
    void start();

    /**
     * @name: stop
     * @Descripttion: stop pipeline, will release dag info, node info, etc.., so if you want reuse it. you should add node info again
     * @param {*}
     * @return {*}
     */    
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

    //<notifierType, notfiers>
    std::unordered_map<NotifierType,std::vector<std::shared_ptr<Notifier<Item>>> > m_notifierMaps;

    std::atomic_bool m_isStop = false;
    bool m_pipelineModified = false;

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
    m_notifierMaps[notifier->type()].push_back(notifier);
    if (notifier->type() == NotifierType::FINAL) {
        notifier->start();
    }
}

template<typename Item>
void PipeLine<Item>::addNotifier(typename Notifier<Item>::NotifierCreateInfo createInfo)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (!checkValid()) return;

    auto notifier = Notifier<Item>::builder()
        .setName(createInfo.name)
        ->setProcessCallback(std::move(createInfo.processCallback))
        ->setType(std::move(createInfo.type))
        ->setQueueSize(createInfo.readyQueueSize)
        ->build();

    if (!notifier) {
        VTF_LOGE("create notifier failed. please check notifier buidler's build progress");
        return;
    }
    VTF_LOGD("add notifier type is {0}", notifier->type());
    m_notifierMaps[notifier->type()].push_back(notifier);
    if (notifier->type() == NotifierType::FINAL) {
        notifier->start();
    }
}

template<typename Item>
bool PipeLine<Item>::constructPipelinesByScenario()
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (!checkValid()) return false;
    if (!m_pipelineModified) return true;

    for (auto[notifierType, notifiers] : m_notifierMaps) {
        for (auto notifier : notifiers) {
            m_pipeNodeDispatcher->addNotifier(notifier);
        }
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
    return true;
}

template<typename Item>
void PipeLine<Item>::start()
{
    if (!checkValid()) return;
    VTF_LOGD("pipeline start...");
    m_isStop = false;
    constructPipelinesByScenario();
    m_pipeNodeDispatcher->start();
    VTF_LOGD("pipeline start END");

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

    for (auto[notifierType, notfiers] : m_notifierMaps) {
        for (auto notifier : notfiers) {
            notifier->stop();
        }
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

