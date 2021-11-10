/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 15:32:04
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-07 19:31:36
 */
#pragma once

#include "../dispatcher.hpp"
#include "../blocking_queue.hpp"
#include "pipeNode.hpp"

#include <type_traits>

namespace vtf {
namespace pipeline {

constexpr int defaultQueueSize = 32;

template<typename Item>
class PipeNodeDispatcher : public Dispatcher<Item> {
public:    
    using ItemQueue = BlockingQueue<std::shared_ptr<Item>>;
    using PipeNodeMap = std::unordered_map<long, std::shared_ptr<PipeNode<Item>>>;
    PipeNodeDispatcher(int dispatchQueueSize = defaultQueueSize)
        :m_dispatchQueue(dispatchQueueSize) 
    {}

    ~PipeNodeDispatcher()
    {}

    virtual bool dispatch(std::shared_ptr<Item> item) override;
    virtual void queueInDispacther(std::shared_ptr<Item> item) override;
    virtual bool threadLoop() override;

    void addPipeNode(std::shared_ptr<PipeNode<Item>> pipeNode);

private:
    ItemQueue m_dispatchQueue;
    PipeNodeMap m_pipeNodeMaps;
};

template<typename Item>
bool PipeNodeDispatcher<Item>::dispatch(std::shared_ptr<Item> item)
{
    VTF_LOGD("vtf::dispatch req id({0})", item->ID());
    std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(500));
    return true;
}
template<typename Item>
void PipeNodeDispatcher<Item>::queueInDispacther(std::shared_ptr<Item> item)
{
    VTF_LOGD("vtf::push req id({0})", item->ID());
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
    VTF_LOGD("add pipe node {0}", nodeId);
}

} //pipeline
} //vtf