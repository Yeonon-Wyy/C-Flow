/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-24 15:56:24
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-30 18:44:31
 */
#pragma once

#include <mutex>

#include "../utils.hpp"
#include "../log.hpp"
#include "pipeNode.hpp"
#include "pipeRequest.hpp"

namespace vtf {
namespace pipeline {

class P1Node : public PipeNode<PipeRequest> {

public:
    virtual bool process(std::shared_ptr<PipeRequest> request) override;
private:
    int m_counter = 0;
    std::mutex m_mutex;
};

bool P1Node::process(std::shared_ptr<PipeRequest> request) 
{
    std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
    m_counter++;
    VTF_LOGD("[{0}]p1Node execute...", m_counter);
    if (m_counter == 10) return false;
    return true;
}   

} //namespace pipeline
} //namespace vtf