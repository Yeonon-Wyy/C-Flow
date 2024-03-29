/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-30 15:32:04
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-10-07 19:51:50
 */
#pragma once

#include <atomic>
#include <type_traits>

#include <cflow/common/dispatcher.h>
#include <cflow/common/notifier.h>
#include <cflow/common/scheduler.h>
#include <cflow/utils/threadPool.h>

#include "pipenode.h"

namespace cflow::pipeline {
/**
 * @name: class PipeNodeDispatcher
 * @Descripttion: a dispatcher, will auto dispatch task in pipeline by task
 * dependency.
 */
template <typename Item>
class PipeNodeDispatcher final : public Dispatcher<Item>
{
public:
    using PipeNodeMap =
        std::unordered_map<cflow_id_t, std::weak_ptr<PipeNode<Item>>>;
    PipeNodeDispatcher(int threadPoolSize)
        : Dispatcher<Item>(),
          m_threadPool(threadPoolSize)
    {
    }

    ~PipeNodeDispatcher() { CFLOW_LOGD("dispatch destory"); }

    /**
     * @name: dispatch
     * @Descripttion: dispatch task
     * @param {shared_ptr<Item>} task
     * @return {*}
     */
    bool dispatch(std::shared_ptr<Item> task) override;

    /**
     * @name: queueInDispacther
     * @Descripttion: queue a task to dispatcher
     * @param {shared_ptr<Item>} task
     * @return {*}
     */
    void queueInDispacther(std::shared_ptr<Item> task) override;

    /**
     * @name: threadLoop
     * @Descripttion: thread loop will loop run process function until receive
     * stop flag
     * @param {shared_ptr<Item>} task
     * @return {*}
     */
    bool threadLoop(std::shared_ptr<Item> task) override;

    /**
     * @name: addPipeNode
     * @Descripttion: add pipeNode object to dispacther, note the pipeNode will
     * use weak_ptr to save
     * @param {shared_ptr<PipeNode<Item>>} pipeNode
     * @return {*}
     */
    void addPipeNode(std::shared_ptr<PipeNode<Item>> pipeNode);

    /**
     * @name: getNodeNameByNodeId
     * @Descripttion: just a util function, return a name of given node id.
     * @param {cflow_id_t} nodeId
     * @return {*}
     */
    std::string getNodeNameByNodeId(cflow_id_t nodeId);

    /**
     * @name: stop
     * @Descripttion: stop dispatcher, will stop threadLoop and threadPool.
     * @param {*}
     * @return {*}
     */
    void stop() override;

    /**
     * @name: addNotifier
     * @Descripttion: add a result notifier to dispatcher, note the notifier
     * object will use weak_ptr to save
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
    void notifyNotFinal(std::shared_ptr<Item>, cflow_id_t callerNodeId);

private:
    PipeNodeMap m_pipeNodeMaps;
    std::unordered_map<NotifierType, std::vector<std::weak_ptr<Notifier<Item>>>>
                                     m_notifierMaps;
    cflow::utils::thread::ThreadPool m_threadPool;
};

template <typename Item>
bool PipeNodeDispatcher<Item>::dispatch(std::shared_ptr<Item> task)
{
    CFLOW_LOGD("dispatch task id({0}) nextNodeId {1}", task->ID(),
               task->getCurrentNode());
    // final or pipeline is stoped, will call notifier
    if (task->getCurrentNode() == -1 || m_threadPool.isStoped())
    {
        // just call final notifier
        if (task->getCurrentNode() == -1 && task->getStatus() == TaskStatus::OK)
        {
            notifyFinal(task, NotifyStatus::OK);
        }
        else
        {
            CFLOW_LOGD("task process is occur some error? notify error");
            notifyFinal(task, NotifyStatus::ERROR);
        }
        return true;
    }

    if (task->checkDependencyIsReady())
    {
        cflow_id_t currentProcessNodeId = task->getCurrentNode();
        if (currentProcessNodeId < 0)
        {
            // finish
            return true;
        }
        if (m_pipeNodeMaps.count(currentProcessNodeId) == 0)
        {
            CFLOW_LOGE("can't find currentNode node {0}", currentProcessNodeId);
            return true;
        }

        auto currentNodeSp = m_pipeNodeMaps[currentProcessNodeId].lock();
        // submit to thread pool
        if (currentNodeSp && !m_threadPool.isStoped() &&
            !currentNodeSp->isStop())
        {
            m_threadPool.emplace(&PipeNode<Item>::process, currentNodeSp, task);
            // currentNodeSp->submit(task);
        }
        else
        {
            // if node already stop or destory, should notify error
            notifyFinal(task, NotifyStatus::ERROR);
        }
    }
    return true;
}
template <typename Item>
void PipeNodeDispatcher<Item>::queueInDispacther(std::shared_ptr<Item> task)
{
    ThreadLoop<std::shared_ptr<Item>, Scheduler>::queueItem(task);
    CFLOW_LOGD("queue task id({0})", task->ID());
    return;
}

template <typename Item>
bool PipeNodeDispatcher<Item>::threadLoop(std::shared_ptr<Item> task)
{
    bool ret = true;
    ret      = dispatch(task);
    return ret;
}

template <typename Item>
void PipeNodeDispatcher<Item>::addPipeNode(
    std::shared_ptr<PipeNode<Item>> pipeNode)
{
    cflow_id_t nodeId = pipeNode->getNodeId();
    if (m_pipeNodeMaps.count(nodeId) == 0)
    {
        m_pipeNodeMaps[nodeId] = pipeNode;
    }
    CFLOW_LOGD("add a pipe node [{0}:{1}]", nodeId, pipeNode->name());
}

template <typename Item>
std::string PipeNodeDispatcher<Item>::getNodeNameByNodeId(cflow_id_t nodeId)
{
    if (m_pipeNodeMaps.count(nodeId) > 0)
    {
        auto nodeSp = m_pipeNodeMaps[nodeId].lock();
        if (nodeSp) return nodeSp->name();
    }
    return "";
}

template <typename Item>
void PipeNodeDispatcher<Item>::stop()
{
    CFLOW_LOGD("pipeNodeDispatcher stop start");
    m_threadPool.stop();
    ThreadLoop<std::shared_ptr<Item>, Scheduler>::stop();
    m_pipeNodeMaps.clear();
    m_notifierMaps.clear();
    CFLOW_LOGD("pipeNodeDispatcher stop end");
}

template <typename Item>
void PipeNodeDispatcher<Item>::notifyFinal(std::shared_ptr<Item> task,
                                           NotifyStatus          status)
{
    if (m_notifierMaps.count(NotifierType::FINAL))
    {
        auto notfiers = m_notifierMaps[NotifierType::FINAL];
        for (auto notifier : notfiers)
        {
            auto notifierSp = notifier.lock();
            task->setNotifyStatus(std::move(status));
            if (notifierSp)
            {
                notifierSp->notify(task);
            }
        }
    }
}

template <typename Item>
void PipeNodeDispatcher<Item>::notifyNotFinal(std::shared_ptr<Item> task,
                                              cflow_id_t callerNodeId)
{
    std::vector<cflow_id_t> notifierIdsForCurrentItem =
        task->getNotifiersByNodeId(callerNodeId);
    for (auto [notifierType, notifiers] : m_notifierMaps)
    {
        // foreach all notifiers
        if (notifierType != NotifierType::FINAL)
        {
            // if type notifier is not final
            for (auto notifier : notifiers)
            {
                // foreach notifiers of someone type
                auto notifierSp = notifier.lock();
                if (m_threadPool.isStoped() ||
                    task->getStatus() == TaskStatus::ERROR)
                {
                    task->setNotifyStatus(NotifyStatus::ERROR);
                }
                if (notifierSp)
                {
                    cflow_id_t notifierId = notifierSp->ID();
                    // find current task and node notifier
                    auto it =
                        std::find(notifierIdsForCurrentItem.begin(),
                                  notifierIdsForCurrentItem.end(), notifierId);
                    if (it != notifierIdsForCurrentItem.end())
                    {
                        notifierSp->notify(task);
                    }
                }
            }
        }
    }
}
} // namespace cflow::pipeline