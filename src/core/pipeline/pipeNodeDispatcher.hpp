/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 15:32:04
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-20 17:44:50
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

constexpr int defaultQueueSize = 32;
constexpr int defaultThreadPoolSize = 8;

template<typename Item>
class PipeNodeDispatcher : public Dispatcher<Item> {
public:    
    using ItemQueue = BlockingQueue<std::shared_ptr<Item>>;
    using PipeNodeMap = std::unordered_map<long, std::weak_ptr<PipeNode<Item>>>;
    PipeNodeDispatcher(int dispatchQueueSize = defaultQueueSize)
        :m_dispatchQueue(dispatchQueueSize),
         m_threadPool(defaultThreadPoolSize),
         m_isStop(false)
    {}

    ~PipeNodeDispatcher()
    {
        VTF_LOGD("dispatch destory");
    }

    virtual bool dispatch(std::shared_ptr<Item> item) override;
    virtual void queueInDispacther(std::shared_ptr<Item> item) override;
    virtual bool threadLoop() override;
    void addPipeNode(std::shared_ptr<PipeNode<Item>> pipeNode);
    std::string getNodeNameByNodeId(long nodeId);

    void stop() override;
    void addResultNotifier(std::shared_ptr<Notifier<Item>> notifier)
    {
        m_resultNotifiers.push_back(notifier);
    }

private:
    ItemQueue m_dispatchQueue;
    PipeNodeMap m_pipeNodeMaps;
    std::vector<std::shared_ptr<Notifier<Item>>> m_resultNotifiers;
    vtf::ThreadPool m_threadPool;
    std::atomic_bool m_isStop;
};

template<typename Item>
bool PipeNodeDispatcher<Item>::dispatch(std::shared_ptr<Item> item)
{
    VTF_LOGD("dispatch item id({0}) nextNodeId {1}, notifier size ({2})", item->ID(), item->getCurrentNode(), m_resultNotifiers.size());
    if (item->getCurrentNode() == -1) {
        for (auto& notifier : m_resultNotifiers) {
            notifier->notify(item);
        }
        return true;
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
        if (currentNodeSp)
            m_threadPool.emplace(&PipeNode<Item>::submit, currentNodeSp, item);
    }
    return true;
}
template<typename Item>
void PipeNodeDispatcher<Item>::queueInDispacther(std::shared_ptr<Item> item)
{
    VTF_LOGD("queue req id({0})", item->ID());
    m_dispatchQueue.push(item);
    return;
}

template<typename Item>
bool PipeNodeDispatcher<Item>::threadLoop()
{
    bool ret = true;
    auto req = m_dispatchQueue.pop();
    ret = dispatch(req);
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
}

} //pipeline
} //vtf