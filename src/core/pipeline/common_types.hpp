/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-10 20:52:24
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-23 22:36:35
 */
#pragma once

#include <condition_variable>
#include <mutex>

namespace vtf {
namespace pipeline {

using PipelineScenario = uint32_t;

enum PipeNodeStatus {
    PROCESSING,
    IDLE,
    STOP,
};

static std::condition_variable g_pipeNodeStopCV;
static std::mutex g_pipeNodeStopMutex;

}
}