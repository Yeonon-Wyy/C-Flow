/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 15:32:04
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-16 22:52:29
 */
#pragma once

#include "../dispatcher.hpp"
#include "../blocking_queue.hpp"
#include "../threadPool.hpp"
#include "pipeNode.hpp"

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
    using PipeNodeMap = std::unordered_map<long, std::shared_ptr<PipeNode<Item>>>;
    PipeNodeDispatcher(int dispatchQueueSize = defaultQueueSize)
        :m_dispatchQueue(dispatchQueueSize),
         m_threadPool(defaultThreadPoolSize),
         m_isStop(false)
    {}

    ~PipeNodeDispatcher()
    {}

    virtual bool dispatch(std::shared_ptr<Item> item) override;
    virtual void queueInDispacther(std::shared_ptr<Item> item) override;
    virtual bool threadLoop() override;
    void addPipeNode(std::shared_ptr<PipeNode<Item>> pipeNode);
    std::string getNodeNameByNodeId(long nodeId);

    void stop();

private:
    ItemQueue m_dispatchQueue;
    PipeNodeMap m_pipeNodeMaps;
    vtf::ThreadPool m_threadPool;
    std::atomic_bool m_isStop;
};

template<typename Item>
bool PipeNodeDispatcher<Item>::dispatch(std::shared_ptr<Item> item)
{
    if (m_isStop) {
        return false;
    }
    VTF_LOGD("dispatch item id({0})", item->ID());
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

        auto currentNode = m_pipeNodeMaps[currentProcessNodeId];
        //submit to thread pool
        m_threadPool.emplace(&PipeNode<Item>::submit, currentNode, item);
    
    }
    return true;
}
template<typename Item>
void PipeNodeDispatcher<Item>::queueInDispacther(std::shared_ptr<Item> item)
{
    if (m_isStop) {
        return;
    }
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
        return m_pipeNodeMaps[nodeId]->name();
    }
    return "";
}

template<typename Item>
void PipeNodeDispatcher<Item>::stop()
{
    m_isStop = true;
    VTF_LOGD("pipeline dispacther clear START");
    for (auto&[nodeId, node] : m_pipeNodeMaps) {
        node->stop();
    }
    {
        std::unique_lock<std::mutex> lk(g_pipeNodeStopMutex);
        g_pipeNodeStopCV.wait(lk, [this]() {
            for (auto&[nodeId, node] : m_pipeNodeMaps) {
                if (node->status() == PipeNodeStatus::PROCESSING) {
                    VTF_LOGD("node [{0}:{1}] still processing", nodeId, node->name());
                    return false;
                }
            }
            return true;
        });
    }

    VTF_LOGD("pipeline dispacther stop END");
    m_dispatchQueue.clear();
}

} //pipeline
} //vtf