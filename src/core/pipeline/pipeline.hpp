/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 18:48:53
 * @LastEditors: yeonon
 * @LastEditTime: 2021-12-18 17:45:30
 */

#pragma once

#include "../dag.hpp"
#include "pipenode.hpp"
#include "pipenode_dispatcher.hpp"
#include "../notifier.hpp"
#include "../log.hpp"

#include <memory>
#include <mutex>
#include <vector>
#include <unordered_map>

namespace vtf {
namespace pipeline {

/**
 * @name: class PipeLine
 * @Descripttion: unified entry as a business, manage all resource(include dispatcher, node, notifier, etc..) and their life cycles
 *                use only need call "submit" to submit a data, pipelien will auto construct denpendcy with scenurio. and run it until stop
 * @param {*}
 * @return {*}
 */
template<typename Item>
class PipeLine {
public:
    using ProcessFunction = std::function<bool(std::shared_ptr<Item>)>;
    using GraphType = std::unordered_map<long, std::vector<long>>;

    struct ConfigureTable {
        int queueSize = 8;
        int threadPoolSize = 8;
        std::vector<typename PipeNode<Item>::PipeNodeCreateInfo> pipeNodeCreateInfos;
        std::vector<typename Notifier<Item>::NotifierCreateInfo> notifierCreateInfos;
        std::vector<std::pair<long, long>> nodeConnections;
    };

public:
    PipeLine(long queueSize, int threadPoolSize)
        :m_dag(),
         m_pipeNodeDispatcher(std::make_shared<PipeNodeDispatcher<Item>>(queueSize, threadPoolSize))
    {}

    ~PipeLine()
    {
        //Because the life cycle of these classes(pipeNode,pipeNodeDispatcher,notifier) is controlled by pipeLine,
        //So, we only need call stop in pipeline's destructor, will not call "stop" of these classese.
        if (!m_isStop)
            stop();
    }

    static std::shared_ptr<PipeLine<Item>> generatePipeLineByConfigureTable(const ConfigureTable&);

    /**
     * @name: addPipeNode
     * @Descripttion: add a pipe node to pipeline. pipeline will generate a new node object for user.
     * @param {ProcessFunction&&} pf is node process function.
     * @return {*}
     */    
    std::shared_ptr<PipeNode<Item>> addPipeNode(typename PipeNode<Item>::PipeNodeCreateInfo createInfo);

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
     * @name: submit a data to pipeline
     * @Descripttion: 
     * @param {shared_ptr<Item>} data
     * @return {*}
     */    
    bool submit(std::shared_ptr<Item> data);

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

    /**
     * @name: checkValid
     * @Descripttion: check pipeline is valid
     * @param {*}
     * @return {*}
     */
    bool checkValid();

    void connectNode(long src, long dst);

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
std::shared_ptr<PipeLine<Item>> PipeLine<Item>::generatePipeLineByConfigureTable(const ConfigureTable& configTable)
{
    std::shared_ptr<PipeLine<Item>> ppl = std::make_shared<PipeLine<Item>>(configTable.queueSize, configTable.threadPoolSize);
    for (auto pipeNodeCreateInfo : configTable.pipeNodeCreateInfos) {
        ppl->addPipeNode(pipeNodeCreateInfo);
    }
    VTF_LOGD("pipeline add pipe node finish");
    for (auto notifierCreateInfo : configTable.notifierCreateInfos) {
        ppl->addNotifier(notifierCreateInfo);
    }
    VTF_LOGD("pipeline add notifier finish");
    for (auto [srcNodeId, dstNodeId] : configTable.nodeConnections) {
        ppl->connectNode(srcNodeId, dstNodeId);
    }
    VTF_LOGD("pipeline connect node finish");
    return ppl;
}

template<typename Item>
bool PipeLine<Item>::checkValid()
{
    if (m_isStop) {
        VTF_LOGD("pipeline already stoped, can't submit any data, please construct another pipeline obejct!!!!");
        return false;
    }
    return true;
}

template<typename Item>
void PipeLine<Item>::connectNode(long src, long dst)
{
    VTF_LOGD("connectNode src {0} dst {1}", src, dst);
    if (m_pipeNodeMaps.count(src) == 0 || m_pipeNodeMaps.count(dst) == 0) {
        VTF_LOGE("can't find src node {0} info or dst node {1} info, please check it.", src, dst);
        return;
    }

    m_pipeNodeMaps[src]->connect(m_pipeNodeMaps[dst]);
}

template<typename Item>
std::shared_ptr<PipeNode<Item>> PipeLine<Item>::addPipeNode(typename PipeNode<Item>::PipeNodeCreateInfo createInfo)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (!checkValid()) return nullptr;


    auto node = PipeNode<Item>::builder()
                .setName(createInfo.name)
                ->setID(std::move(createInfo.id))
                ->setProcessCallback(std::move(createInfo.processCallback))
                ->setConfigProgress(std::move(createInfo.configProgress))
                ->setStopProgress(std::move(createInfo.stopProgress))
                ->addScenarios(createInfo.pipelineScenarios)
                ->build(m_pipeNodeDispatcher);

    node->config();
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
    node->config();
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
    m_pipeNodeDispatcher->addNotifier(notifier);
    notifier->config();
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
        ->setConfigProgress(std::move(createInfo.configProgress))
        ->setStopProgress(std::move(createInfo.stopProgress))
        ->setType(std::move(createInfo.type))
        ->setQueueSize(createInfo.readyQueueSize)
        ->setID(createInfo.id)
        ->build();

    if (!notifier) {
        VTF_LOGE("create notifier failed. please check notifier buidler's build progress");
        return;
    }
    VTF_LOGD("add notifier type is {0}", notifier->type());
    m_pipeNodeDispatcher->addNotifier(notifier);
    m_notifierMaps[notifier->type()].push_back(notifier);
    notifier->config();
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

    //get all scenario
    for (auto&[nodeId, node] : m_pipeNodeMaps) {
        for (PipelineScenario scenario : node->getScenarios()) {
            m_pipelineScenarioSet.insert(scenario);
        }
    }
    auto pipelines = m_dag.multiTopologicalSort();

    for (auto it = m_pipelineScenarioSet.begin(); it != m_pipelineScenarioSet.end(); it++) {
        PipelineScenario scenario = *it;
        VTF_LOGD("scenario {0} ", scenario);
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
bool PipeLine<Item>::submit(std::shared_ptr<Item> data)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (!checkValid()) return false;
    data->constructDependency(getPipelineWithScenario(data->scenario()));
    m_pipeNodeDispatcher->queueInDispacther(data);
    VTF_LOGD("submit a data {0}", data->ID());
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

    for (auto&[nodeId, node] : m_pipeNodeMaps) {
        node->stop();
    }
    m_pipeNodeDispatcher->stop();

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

