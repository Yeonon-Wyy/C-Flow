/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 15:32:04
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-28 18:37:57
 */
#pragma once

#include "../dispatcher.hpp"
#include "../blocking_queue.hpp"
#include "../threadPool.hpp"
#include "pipeNode.hpp"
#include "../notifier.hpp"

#include <type_traits>
#include <atomic>

namespace vtf {
namespace pipeline {

constexpr int defaultThreadPoolSize = 8;

/**
 * @name: class PipeNodeDispatcher
 * @Descripttion: a dispatcher, will auto dispatch data in pipeline by data dependency.
 */
template<typename Item>
class PipeNodeDispatcher : public Dispatcher<Item> {
public:    
    using PipeNodeMap = std::unordered_map<long, std::weak_ptr<PipeNode<Item>>>;
    PipeNodeDispatcher(int dispatchQueueSize = defaultDsiapctherQueueSize)
        :Dispatcher<Item>(dispatchQueueSize),
         m_threadPool(defaultThreadPoolSize)
    {}

    ~PipeNodeDispatcher()
    {
        VTF_LOGD("dispatch destory");
    }

    /**
     * @name: dispatch
     * @Descripttion: dispatch item
     * @param {shared_ptr<Item>} item
     * @return {*}
     */    
    bool dispatch(std::shared_ptr<Item> item) override;

    /**
     * @name: queueInDispacther
     * @Descripttion: queue a item to dispatcher
     * @param {shared_ptr<Item>} item
     * @return {*}
     */    
    void queueInDispacther(std::shared_ptr<Item> item) override;

    /**
     * @name: threadLoop
     * @Descripttion: thread loop will loop run process function until receive stop flag
     * @param {shared_ptr<Item>} item
     * @return {*}
     */    
    bool threadLoop(std::shared_ptr<Item> item) override;

    /**
     * @name: addPipeNode
     * @Descripttion: add pipeNode object to dispacther, note the pipeNode will use weak_ptr to save
     * @param {shared_ptr<PipeNode<Item>>} pipeNode
     * @return {*}
     */    
    void addPipeNode(std::shared_ptr<PipeNode<Item>> pipeNode);

    /**
     * @name: getNodeNameByNodeId
     * @Descripttion: just a util function, return a name of given node id.
     * @param {long} nodeId
     * @return {*}
     */    
    std::string getNodeNameByNodeId(long nodeId);

    /**
     * @name: stop
     * @Descripttion: stop dispatcher, will stop threadLoop and threadPool.
     * @param {*}
     * @return {*}
     */    
    void stop() override;

    /**
     * @name: addNotifier
     * @Descripttion: add a result notifier to dispatcher, note the notifier object will use weak_ptr to save
     * @param {shared_ptr<Notifier<Item>>} notifier
     * @return {*}
     */    
    void addNotifier(std::shared_ptr<Notifier<Item>> notifier)
    {
        m_notifierMaps[notifier->type()].push_back(notifier);
    }

private:
    PipeNodeMap m_pipeNodeMaps;
    std::unordered_map<NotifierType, std::vector<std::weak_ptr<Notifier<Item>>> > m_notifierMaps;
    vtf::ThreadPool m_threadPool;
};

template<typename Item>
bool PipeNodeDispatcher<Item>::dispatch(std::shared_ptr<Item> item)
{    
    VTF_LOGD("dispatch item id({0}) nextNodeId {1} m_notifierMaps.size ({2})", item->ID(), item->getCurrentNode(), m_notifierMaps.size());
    //final or pipeline is stoped, will call notifier
    if (item->getCurrentNode() == -1 || m_threadPool.isStoped()) {
        //just call final notifier
        if (m_notifierMaps.count(NotifierType::FINAL)) {
            auto notfiers = m_notifierMaps[NotifierType::FINAL];
            for (auto notifier : notfiers) {
                auto notifierSp = notifier.lock();
                if (m_threadPool.isStoped()) {
                    item->setNotifyStatus(NotifyStatus::ERROR);
                }
                if (notifierSp) {
                    notifierSp->notify(item);
                }
            }
        }
        return true;
    } else {
        for (auto[notifierType, notifiers] : m_notifierMaps) {
            if (notifierType != NotifierType::FINAL) {
                for (auto notifier : notifiers) {
                    auto notifierSp = notifier.lock();
                    if (m_threadPool.isStoped()) {
                        item->setNotifyStatus(NotifyStatus::ERROR);
                    }
                    if (notifierSp) {
                        notifierSp->notify(item);
                    }
                }
            }
        }
    }

    if (item->checkDependencyIsReady()) {
        long currentProcessNodeId = item->getCurrentNode();
        
        if (currentProcessNodeId < 0) {
            //finish
            return true;
        }
        if (m_pipeNodeMaps.count(currentProcessNodeId) == 0) {
            VTF_LOGE("can't find currentNode node {0}", currentProcessNodeId);
            return true;
        }

        auto currentNodeSp = m_pipeNodeMaps[currentProcessNodeId].lock();
        //submit to thread pool
        if (currentNodeSp && !m_threadPool.isStoped()) {
            VTF_LOGD("node process {0}", item->ID());
            m_threadPool.emplace(&PipeNode<Item>::process, currentNodeSp, item);
        }
    }
    return true;
}
template<typename Item>
void PipeNodeDispatcher<Item>::queueInDispacther(std::shared_ptr<Item> item)
{
    ThreadLoop<std::shared_ptr<Item>>::queueItem(item);
    VTF_LOGD("queue item id({0})", item->ID());
    return;
}

template<typename Item>
bool PipeNodeDispatcher<Item>::threadLoop(std::shared_ptr<Item> item)
{
    bool ret = true;
    ret = dispatch(item);
    return ret;
}

template<typename Item>
void PipeNodeDispatcher<Item>::addPipeNode(std::shared_ptr<PipeNode<Item>> pipeNode)
{
    long nodeId = pipeNode->getNodeId();
    if (m_pipeNodeMaps.count(nodeId) == 0) {
        m_pipeNodeMaps[nodeId] = pipeNode;
    }
    VTF_LOGD("add a pipe node [{0}:{1}]", nodeId, pipeNode->name());
}

template<typename Item>
std::string PipeNodeDispatcher<Item>::getNodeNameByNodeId(long nodeId)
{
    if (m_pipeNodeMaps.count(nodeId) > 0) {
        auto nodeSp = m_pipeNodeMaps[nodeId].lock();
        if (nodeSp)
            return nodeSp->name();
    }
    return "";
}

template<typename Item>
void PipeNodeDispatcher<Item>::stop()
{
    VTF_LOGD("pipeNodeDispatcher stop start");
    m_threadPool.stop();
    ThreadLoop<std::shared_ptr<Item>>::stop();
    m_pipeNodeMaps.clear();
    m_notifierMaps.clear();
    VTF_LOGD("pipeNodeDispatcher stop end");
}

} //pipeline
} //vtf