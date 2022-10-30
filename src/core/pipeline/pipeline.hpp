/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-30 18:48:53
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-10-30 19:59:01
 */

#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <fstream>

#include "../dag.hpp"
#include "../notifier.hpp"
#include "../utils/log/log.hpp"
#include "../utils/memory/buffer_manager.hpp"
#include "../utils/memory/buffer_manager_factory.hpp"
#include "../utils/dumper.hpp"
#include "pipenode.hpp"
#include "pipenode_dispatcher.hpp"

namespace cflow::pipeline {
using namespace cflow::utils::memory;

const static int pplDefaultMaxProcessingSize = 32;
// some CPU architectures are 4 core 8 threads(like intel), _SC_NPROCESSORS_CONF
// can't get cpu thread number for common, pplDefaultThreadPoolSize just use CPU
// core number + 1, not thread numbers
const static int pplDefaultThreadPoolSize = sysconf(_SC_NPROCESSORS_CONF) + 1;

/**
 * @name: class PipeLine
 * @Descripttion: unified entry as a business, manage all resource(include
 * dispatcher, node, notifier, etc..) and their life cycles use only need call
 * "submit" to submit a task, pipelien will auto construct denpendcy with
 * scenurio. and run it until stop
 * @param {*}
 * @return {*}
 */
template <typename Item>
class PipeLine
{
public:
    using ProcessFunction = std::function<bool(std::shared_ptr<Item>)>;

    struct ConfigureTable
    {
        int maxProcessingSize = pplDefaultMaxProcessingSize;
        int threadPoolSize    = pplDefaultThreadPoolSize;
        std::vector<typename PipeNode<Item>::PipeNodeCreateInfo>
            pipeNodeCreateInfos;
        std::vector<typename Notifier<Item>::NotifierCreateInfo>
                                                       notifierCreateInfos;
        std::vector<std::pair<cflow_id_t, cflow_id_t>> nodeConnections;
    };

public:
    /**
     * @name: PipeLine
     * @Descripttion: PipeLine constructor
     * @param {int} maxProcessingSize
     * @param {int} threadPoolSize
     * @return {*}
     */
    PipeLine(int maxProcessingSize, int threadPoolSize, bool dumpGraph = false)
        : m_dag(),
          m_pipeNodeDispatcher(
              std::make_shared<PipeNodeDispatcher<Item>>(threadPoolSize)),
          m_bufferMgrFactory(std::make_shared<BufferManagerFactory<void>>()),
          m_processingTaskCount(0),
          m_processingMaxTaskCount(maxProcessingSize),
          m_dumpGraph(dumpGraph)
    {
    }

    ~PipeLine()
    {
        // Because the life cycle of these
        // classes(pipeNode,pipeNodeDispatcher,notifier) is controlled by
        // pipeLine, So, we only need call stop in pipeline's destructor, will
        // not call "stop" of these classese.
        if (!m_isStop) stop();
    }

    static std::shared_ptr<PipeLine<Item>> generatePipeLineByConfigureTable(
        const ConfigureTable&);

    /**
     * @name: addPipeNode
     * @Descripttion: add a pipe node to pipeline. pipeline will generate a new
     * node object for user.
     * @param {ProcessFunction&&} pf is node process function.
     * @return {*}
     */
    std::shared_ptr<PipeNode<Item>> addPipeNode(
        typename PipeNode<Item>::PipeNodeCreateInfo createInfo);

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
     * @Descripttion: add a notifier by create infos, will return a notifier
     * object
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
     * @name: notifyDone
     * @Descripttion: register this function to final notifier, final notifier
     * will call it when item process done
     * @param {*}
     * @return {*}
     */
    void notifyDone(cflow_id_t id);

    /**
     * @name: submit a task to pipeline
     * @Descripttion:
     * @param {shared_ptr<Item>} task
     * @return {*}
     */
    bool submit(std::shared_ptr<Item> task);

    /**
     * @name: start
     * @Descripttion: start pipeline, must call it
     * @param {*}
     * @return {*}
     */
    void start();

    /**
     * @name: stop
     * @Descripttion: stop pipeline, will release dag info, node info, etc.., so
     * if you want reuse it. you should add node info again
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
    std::vector<cflow_id_t> getPipelineWithScenario(PipelineScenario scenario);

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

    void connectNode(cflow_id_t src, cflow_id_t dst);

private:
    DAG                                       m_dag;
    std::shared_ptr<PipeNodeDispatcher<Item>> m_pipeNodeDispatcher;
    std::unordered_map<cflow_id_t, std::shared_ptr<PipeNode<Item>>>
        m_pipeNodeMaps;
    std::unordered_map<PipelineScenario, std::vector<cflow_id_t>>
                                              m_scenario2PipelineMaps;
    std::unordered_map<PipelineScenario, int> m_pipelineScenarioMap;
    //<notifierType, notfiers>
    std::unordered_map<NotifierType,
                       std::vector<std::shared_ptr<Notifier<Item>>>>
                                                m_notifierMaps;
    std::atomic_bool                            m_isStop           = false;
    bool                                        m_pipelineModified = false;
    std::shared_ptr<BufferManagerFactory<void>> m_bufferMgrFactory;
    std::atomic_uint32_t                        m_processingTaskCount;
    std::atomic_uint32_t                        m_processingMaxTaskCount;
    std::condition_variable                     m_processingTaskCV;
    std::mutex                                  m_mutex;

    // for debug
    bool m_dumpGraph = false;
};

template <typename Item>
std::shared_ptr<PipeLine<Item>>
PipeLine<Item>::generatePipeLineByConfigureTable(
    const ConfigureTable& configTable)
{
    std::shared_ptr<PipeLine<Item>> ppl = std::make_shared<PipeLine<Item>>(
        configTable.maxProcessingSize, configTable.threadPoolSize);
    for (auto pipeNodeCreateInfo : configTable.pipeNodeCreateInfos)
    {
        ppl->addPipeNode(pipeNodeCreateInfo);
    }
    CFLOW_LOGD("pipeline add pipe node finish");
    for (auto notifierCreateInfo : configTable.notifierCreateInfos)
    {
        ppl->addNotifier(notifierCreateInfo);
    }
    CFLOW_LOGD("pipeline add notifier finish");
    for (auto [srcNodeId, dstNodeId] : configTable.nodeConnections)
    {
        ppl->connectNode(srcNodeId, dstNodeId);
    }
    CFLOW_LOGD("pipeline connect node finish");
    return ppl;
}

template <typename Item>
bool PipeLine<Item>::checkValid()
{
    if (m_isStop)
    {
        CFLOW_LOGD("pipeline already stoped, can't submit any task, please "
                   "construct another pipeline obejct!!!!");
        return false;
    }
    return true;
}

template <typename Item>
void PipeLine<Item>::connectNode(cflow_id_t src, cflow_id_t dst)
{
    CFLOW_LOGD("connectNode src {0} dst {1}", src, dst);
    if (m_pipeNodeMaps.count(src) == 0 || m_pipeNodeMaps.count(dst) == 0)
    {
        CFLOW_LOGE("can't find src node {0} info or dst node {1} info, please "
                   "check it.",
                   src, dst);
        return;
    }

    m_pipeNodeMaps[src]->connect(m_pipeNodeMaps[dst]);
}

template <typename Item>
std::shared_ptr<PipeNode<Item>> PipeLine<Item>::addPipeNode(
    typename PipeNode<Item>::PipeNodeCreateInfo createInfo)
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
template <typename Item>
bool PipeLine<Item>::addPipeNode(std::shared_ptr<PipeNode<Item>> node)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (!checkValid()) return false;
    if (m_pipeNodeMaps.count(node->getNodeId()) != 0)
    {
        CFLOW_LOGE("node [{0}:{1}] already be added to pipeline. don't need "
                   "add again.",
                   node->getNodeId(), node->name());
        return false;
    }
    node->config();
    m_pipeNodeMaps[node->getNodeId()] = node;
    m_dag.addNode(node);
    m_pipeNodeDispatcher->addPipeNode(node);
    m_pipelineModified = true;
    return true;
}

template <typename Item>
void PipeLine<Item>::addNotifier(std::shared_ptr<Notifier<Item>> notifier)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (!checkValid()) return;
    m_notifierMaps[notifier->type()].push_back(notifier);
    m_pipeNodeDispatcher->addNotifier(notifier);
    notifier->config();
    if (notifier->type() == NotifierType::FINAL)
    {
        notifier->start();
    }
}

template <typename Item>
void PipeLine<Item>::addNotifier(
    typename Notifier<Item>::NotifierCreateInfo createInfo)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (!checkValid()) return;

    // Note: setNotifyDoneCallback must be after setType
    auto notifier =
        Notifier<Item>::builder()
            .setName(createInfo.name)
            ->setType(std::move(createInfo.type))
            ->setProcessCallback(std::move(createInfo.processCallback))
            ->setNotifyDoneCallback(std::bind(&PipeLine<Item>::notifyDone, this,
                                              std::placeholders::_1))
            ->setConfigProgress(std::move(createInfo.configProgress))
            ->setStopProgress(std::move(createInfo.stopProgress))
            ->setID(createInfo.id)
            ->build();

    if (!notifier)
    {
        CFLOW_LOGE("create notifier failed. please check notifier buidler's "
                   "build progress");
        return;
    }
    CFLOW_LOGD("add notifier type is {0}", notifier->type());
    m_pipeNodeDispatcher->addNotifier(notifier);
    m_notifierMaps[notifier->type()].push_back(notifier);
    notifier->config();
    if (notifier->type() == NotifierType::FINAL)
    {
        notifier->start();
    }
}

template <typename Item>
bool PipeLine<Item>::constructPipelinesByScenario()
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (!checkValid()) return false;
    if (!m_pipelineModified) return true;

    // 0 -> 100, 101, 102
    // 1 -> 100, 102
    // 2 -> 101
    // 3 -> 100, 101

    auto pipelines = m_dag.multiTopologicalSort();
    // get all node in pipeline, maybe not equal all graph node.
    std::unordered_set<cflow_id_t> pipelineNodeIds;
    for (auto&& pipeline : pipelines)
    {
        for (auto&& nodeId : pipeline)
        {
            pipelineNodeIds.insert(nodeId);
        }
    }

    // get all scenario
    for (auto nodeId : pipelineNodeIds)
    {
        if (m_pipeNodeMaps.count(nodeId) == 0) continue;
        for (PipelineScenario scenario : m_pipeNodeMaps[nodeId]->getScenarios())
        {
            m_pipelineScenarioMap[scenario]++;
        }
    }

    // construct pipeline for all scenario
    for (auto it = m_pipelineScenarioMap.begin();
         it != m_pipelineScenarioMap.end(); it++)
    {
        PipelineScenario scenario         = it->first;
        size_t           scenarioNodeSize = it->second;
        CFLOW_LOGD("current construct pipeline for scenario {0} ", scenario);
        for (auto pipelineIter = pipelines.begin();
             pipelineIter != pipelines.end(); pipelineIter++)
        {
            auto                    pipeline = *pipelineIter;
            std::vector<cflow_id_t> nodeConnections;
            for (cflow_id_t& nodeID : pipeline)
            {
                if (m_pipeNodeMaps.count(nodeID) == 0)
                {
                    CFLOW_LOGE("can't find node {0} in m_pipeNodeMaps, please "
                               "check it.",
                               nodeID);
                    m_scenario2PipelineMaps.clear();
                    return false;
                }
                if (m_pipeNodeMaps[nodeID]->hasScenario(scenario))
                {
                    nodeConnections.push_back(nodeID);
                }
            }

            if (nodeConnections.size() == scenarioNodeSize)
            {
                // check node connection
                bool isConnect = true;
                for (size_t i = 1; i < nodeConnections.size(); i++)
                {
                    if (!m_dag.checkDependency(nodeConnections[i - 1],
                                               nodeConnections[i]))
                    {
                        isConnect = false;
                        break;
                    }
                }
                if (isConnect)
                {
                    if (m_scenario2PipelineMaps.count(scenario))
                    {
                        CFLOW_LOGE("already have a pieline of scenario {0}, "
                                   "new pipeline will cover old pipeline, "
                                   "something error???",
                                   scenario);
                    }
                    m_scenario2PipelineMaps[scenario] = nodeConnections;
                }
            }
        }
    }

    m_pipelineModified = false;
    dumpPipelines();
    return true;
}

template <typename Item>
void PipeLine<Item>::notifyDone(cflow_id_t id)
{
    m_processingTaskCount--;
    m_processingTaskCV.notify_one();
}

template <typename Item>
std::vector<cflow_id_t> PipeLine<Item>::getPipelineWithScenario(
    PipelineScenario scenario)
{
    if (!checkValid()) return {};
    std::vector<cflow_id_t> pipeline;
    if (m_scenario2PipelineMaps.count(scenario) == 0)
    {
        CFLOW_LOGE("can't find any pipelie about scenario {0}", scenario);
    }
    else
    {
        pipeline = m_scenario2PipelineMaps[scenario];
    }

    return pipeline;
}

template <typename Item>
bool PipeLine<Item>::submit(std::shared_ptr<Item> task)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (!checkValid()) return false;
    CFLOW_LOGD("submit a task {0}", task->ID());
    auto pipeline = getPipelineWithScenario(task->scenario());
    task->constructDependency(pipeline, m_bufferMgrFactory);
    m_pipeNodeDispatcher->queueInDispacther(task);
    if (m_processingTaskCount >= m_processingMaxTaskCount)
    {
        m_processingTaskCV.wait(lk, [&]() {
            return m_processingTaskCount < m_processingMaxTaskCount;
        });
    }
    m_processingTaskCount++;
    return true;
}

template <typename Item>
void PipeLine<Item>::start()
{
    if (!checkValid()) return;
    CFLOW_LOGD("pipeline start...");
    m_isStop = false;
    constructPipelinesByScenario();
    m_pipeNodeDispatcher->start();
    CFLOW_LOGD("pipeline start END");
}

template <typename Item>
void PipeLine<Item>::stop()
{
    std::unique_lock<std::mutex> lk(m_mutex);
    m_isStop = true;

    for (auto& [nodeId, node] : m_pipeNodeMaps)
    {
        node->stop();
    }
    m_pipeNodeDispatcher->stop();

    for (auto [notifierType, notfiers] : m_notifierMaps)
    {
        for (auto notifier : notfiers)
        {
            notifier->stop();
        }
    }

    // clear info
    m_dag.clear();
    m_pipeNodeMaps.clear();
    m_pipelineModified = true;
}

// private
template <typename Item>
void PipeLine<Item>::dumpPipelines()
{
    if (!checkValid() || !m_dumpGraph) return;

    // dump all
    {
        std::unordered_map<PipelineScenario, std::vector<std::string>>
                                           scenario2PipelineWithNameMaps;
        std::set<std::vector<std::string>> pipelineWithNameSet;
        for (auto& [scenario, pipeline] : m_scenario2PipelineMaps)
        {
            std::vector<std::string> pipelineWithName;
            for (auto&& nodeId : pipeline)
            {
                pipelineWithName.push_back(m_pipeNodeMaps[nodeId]->name());
            }
            scenario2PipelineWithNameMaps[scenario] = pipelineWithName;
        }
        cflow::utils::Dumper dumper(
            "All_Pipeline", scenario2PipelineWithNameMaps, DUMPTYPE::ALL);
        std::string  filename = "all_pipeline.dot";
        std::fstream fs(filename, std::ios::out | std::ios::trunc);
        dumper.dumpDOT(fs);
    }

    // dump each pipeline
    for (auto& [scenario, pipeline] : m_scenario2PipelineMaps)
    {
        std::unordered_map<PipelineScenario, std::vector<std::string>>
                                 scenario2PipelineWithNameMaps;
        std::vector<std::string> pipelineWithName;
        for (auto&& nodeId : pipeline)
        {
            pipelineWithName.push_back(m_pipeNodeMaps[nodeId]->name());
        }
        scenario2PipelineWithNameMaps[scenario] = (pipelineWithName);
        cflow::utils::Dumper dumper("Scenario_" + std::to_string(scenario),
                                    scenario2PipelineWithNameMaps);
        std::string  filename = "pipeline_" + std::to_string(scenario) + ".dot";
        std::fstream fs(filename, std::ios::out | std::ios::trunc);
        dumper.dumpDOT(fs);
    }
}
} // namespace cflow::pipeline
