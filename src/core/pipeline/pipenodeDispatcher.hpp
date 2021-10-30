/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 15:32:04
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-30 18:29:46
 */
#pragma once

#include "../dispatcher.hpp"
#include "../blocking_queue.hpp"
#include "pipeRequest.hpp"

#include <type_traits>

namespace vtf {
namespace pipeline {

constexpr int defaultQueueSize = 32;

template<typename Item>
class PipeNodeDispatcher : public Dispatcher<Item> {
public:
    static_assert(std::is_base_of<Request, Item>::value, "Item must be a descendant of PipeRequest");
    
    using ItemQueue = BlockingQueue<std::shared_ptr<Item>>;
    PipeNodeDispatcher(int dispatchQueueSize = defaultQueueSize)
        :m_dispatchQueue(dispatchQueueSize) 
    {}

    virtual bool dispatch(std::shared_ptr<Item> item) override;
    virtual void queueInDispacther(std::shared_ptr<Item> item) override;
    
    virtual bool threadLoop() override;

private:
    ItemQueue m_dispatchQueue;
    
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

} //pipeline
} //vtf