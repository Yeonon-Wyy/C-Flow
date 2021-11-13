/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-10 20:52:24
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-13 16:51:46
 */
#pragma once

#include <condition_variable>
#include <mutex>

namespace vtf {
namespace pipeline {

enum PipelineScenario {
    SCENARIO_START,
    SAT,
    BOKEH,
    SUPERNS,
    SCENARIO_END
};

enum PipeNodeStatus {
    PROCESSING,
    IDLE,
    STOP,
};

static std::condition_variable g_pipeNodeStopCV;
static std::mutex g_pipeNodeStopMutex;

}
}