/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 15:32:04
 * @LastEditors: yeonon
 * @LastEditTime: 2022-01-29 14:44:57
 */
#pragma once

#include "../dispatcher.hpp"
#include "../utils/thread/threadPool.hpp"
#include "pipenode.hpp"
#include "../notifier.hpp"
#include "../scheduler.hpp"

#include <type_traits>
#include <atomic>

namespace vtf {
namespace pipeline {
/**
 * @name: class PipeNodeDispatcher
 * @Descripttion: a dispatcher, will auto dispatch data in pipeline by data dependency.
 */
template<typename Item>
class PipeNodeDispatcher : public Dispatcher<Item> {
public:    
    using PipeNodeMap = std::unordered_map<vtf_id_t, std::weak_ptr<PipeNode<Item>>>;
    PipeNodeDispatcher(int threadPoolSize)
        :Dispatcher<Item>(),
         m_threadPool(threadPoolSize)
    {}

    ~PipeNodeDispatcher()
    {
        VTF_LOGD("dispatch destory");
    }

    /**
     * @name: dispatch
     * @Descripttion: dispatch data
     * @param {shared_ptr<Item>} data
     * @return {*}
     */    
    bool dispatch(std::shared_ptr<Item> data) override;

    /**
     * @name: queueInDispacther
     * @Descripttion: queue a data to dispatcher
     * @param {shared_ptr<Item>} data
     * @return {*}
     */    
    void queueInDispacther(std::shared_ptr<Item> data) override;

    /**
     * @name: threadLoop
     * @Descripttion: thread loop will loop run process function until receive stop flag
     * @param {shared_ptr<Item>} data
     * @return {*}
     */    
    bool threadLoop(std::shared_ptr<Item> data) override;

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
     * @param {vtf_id_t} nodeId
     * @return {*}
     */    
    std::string getNodeNameByNodeId(vtf_id_t nodeId);

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

    /**
     * @name: notifyFinal
     * @Descripttion: call final notifier
     * @param {*}
     * @return {*}
     */    
    void notifyFinal(std::shared_ptr<Item>, NotifyStatus);

    /**
     * @name: notifyNotFinal
     * @Descripttion: call not final notifier
     * @param {*}
     * @return {*}
     */    
    void notifyNotFinal(std::shared_ptr<Item>, vtf_id_t callerNodeId);

private:
    PipeNodeMap m_pipeNodeMaps;
    std::unordered_map<NotifierType, std::vector<std::weak_ptr<Notifier<Item>>> > m_notifierMaps;
    vtf::ThreadPool m_threadPool;
};

template<typename Item>
bool PipeNodeDispatcher<Item>::dispatch(std::shared_ptr<Item> data)
{    
    VTF_LOGD("dispatch data id({0}) nextNodeId {1}", data->ID(), data->getCurrentNode());
    //final or pipeline is stoped, will call notifier
    if (data->getCurrentNode() == -1 || m_threadPool.isStoped()) {
        //just call final notifier
        if (data->getCurrentNode() == -1) {
            notifyFinal(data, NotifyStatus::OK);
        } else {
            notifyFinal(data, NotifyStatus::ERROR);
        }
        return true;
    }

    if (data->checkDependencyIsReady()) {
        vtf_id_t currentProcessNodeId = data->getCurrentNode();
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
        if (currentNodeSp && !m_threadPool.isStoped() && !currentNodeSp->isStop()) {
            m_threadPool.emplace(&PipeNode<Item>::process, currentNodeSp, data);
            // currentNodeSp->submit(data);
        } else {
            //if node already stop or destory, should notify error
            notifyFinal(data, NotifyStatus::ERROR);
        }
    }
    return true;
}
template<typename Item>
void PipeNodeDispatcher<Item>::queueInDispacther(std::shared_ptr<Item> data)
{
    ThreadLoop<std::shared_ptr<Item>, Scheduler>::queueItem(data);
    VTF_LOGD("queue data id({0})", data->ID());
    return;
}

template<typename Item>
bool PipeNodeDispatcher<Item>::threadLoop(std::shared_ptr<Item> data)
{
    bool ret = true;
    ret = dispatch(data);
    return ret;
}

template<typename Item>
void PipeNodeDispatcher<Item>::addPipeNode(std::shared_ptr<PipeNode<Item>> pipeNode)
{
    vtf_id_t nodeId = pipeNode->getNodeId();
    if (m_pipeNodeMaps.count(nodeId) == 0) {
        m_pipeNodeMaps[nodeId] = pipeNode;
    }
    VTF_LOGD("add a pipe node [{0}:{1}]", nodeId, pipeNode->name());
}

template<typename Item>
std::string PipeNodeDispatcher<Item>::getNodeNameByNodeId(vtf_id_t nodeId)
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
    ThreadLoop<std::shared_ptr<Item>, Scheduler>::stop();
    m_pipeNodeMaps.clear();
    m_notifierMaps.clear();
    VTF_LOGD("pipeNodeDispatcher stop end");
}

template<typename Item>
void PipeNodeDispatcher<Item>::notifyFinal(std::shared_ptr<Item> data, NotifyStatus status)
{
    if (m_notifierMaps.count(NotifierType::FINAL)) {
        auto notfiers = m_notifierMaps[NotifierType::FINAL];
        for (auto notifier : notfiers) {
            auto notifierSp = notifier.lock();
            data->setNotifyStatus(std::move(status));
            if (notifierSp) {
                notifierSp->notify(data);
            }
        }
    }
}

template<typename Item>
void PipeNodeDispatcher<Item>::notifyNotFinal(std::shared_ptr<Item> data, vtf_id_t callerNodeId)
{
    std::vector<vtf_id_t> notifierIdsForCurrentItem = data->getNotifiersByNodeId(callerNodeId);
    for (auto[notifierType, notifiers] : m_notifierMaps) {
        //foreach all notifiers
        if (notifierType != NotifierType::FINAL) {
            //if type notifier is not final
            for (auto notifier : notifiers) {
                //foreach notifiers of someone type
                auto notifierSp = notifier.lock();
                if (m_threadPool.isStoped()) {
                    data->setNotifyStatus(NotifyStatus::ERROR);
                }
                if (notifierSp) {
                    vtf_id_t notifierId = notifierSp->ID();
                    //find current data and node notifier
                    auto it = std::find(notifierIdsForCurrentItem.begin(), notifierIdsForCurrentItem.end(), notifierId);
                    if (it != notifierIdsForCurrentItem.end()) {
                        notifierSp->notify(data);
                    }
                }
            }
        }
    }
}

} //pipeline
} //vtf