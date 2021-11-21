/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-10 20:52:24
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-21 21:00:00
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

enum NotifyStatus {
    OK,
    ERROR
};

static std::condition_variable g_pipeNodeStopCV;
static std::mutex g_pipeNodeStopMutex;

}
}