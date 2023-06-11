/*
 * @Author: Yeonon
 * @Date: 2022-08-21 16:55:49
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-12-25 18:46:05
 * @FilePath: /src/core/utils/log/trace_log.h
 * @Description:
 * Copyright 2022 Yeonon, All Rights Reserved.
 * 2022-08-21 16:55:49
 */
#pragma once

#include <chrono>
#include <iostream>
#include <string>

#include "time_util.h"
#include "log.h"

#define TRACE_FUNC_START(functionName)               \
    cflow::utils::log::TraceLog trace(functionName); \
    trace.start();

#define TRACE_FUNC_ID_START(functionName, dataId)            \
    cflow::utils::log::TraceLog trace(functionName, dataId); \
    trace.start();

#define TRACE_END() trace.stop();
#define TRACE_END_WITH_TIME(duration) duration = trace.stop();

namespace cflow::utils::log {
class TraceLog
{
public:
    TraceLog(const std::string& functionName);
    TraceLog(const std::string& functionName, uint32_t dataId);

    ~TraceLog();

    /**
     * @description: start timer
     * @return {*}
     */
    void start();

    /**
     * @description: stop timer
     * @return {*} return a duration of start
     */
    auto stop() -> std::chrono::milliseconds;

private:
    std::string                                        m_functionName;
    int32_t                                            m_dataId;
    bool                                               m_isStop;
    std::chrono::time_point<std::chrono::steady_clock> m_start;
    std::chrono::time_point<std::chrono::steady_clock> m_end;
};

} // namespace cflow::utils::log