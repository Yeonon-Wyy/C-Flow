/*
 * @Author: Yeonon
 * @Date: 2022-08-21 16:55:49
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-08-21 17:49:58
 * @FilePath: /src/core/utils/log/trace_log.hpp
 * @Description:
 * Copyright 2022 Yeonon, All Rights Reserved.
 * 2022-08-21 16:55:49
 */
#pragma once

#include <chrono>
#include <iostream>
#include <string>

#include "../time_util.hpp"
#include "log.hpp"

#define TRACE_FUNC_START(functionName)             \
    vtf::utils::log::TraceLog trace(functionName); \
    trace.start();

#define TRACE_FUNC_ID_START(functionName, dataId)          \
    vtf::utils::log::TraceLog trace(functionName, dataId); \
    trace.start();

#define TRACE_END() trace.stop();

namespace vtf
{
namespace utils
{
namespace log
{
class TraceLog
{
public:
    TraceLog(const std::string& functionName);
    TraceLog(const std::string& functionName, uint32_t dataId);

    ~TraceLog();

    void start();
    void stop();

private:
    std::string                                        m_functionName;
    int32_t                                            m_dataId;
    bool                                               m_isStop;
    std::chrono::time_point<std::chrono::steady_clock> m_start;
    std::chrono::time_point<std::chrono::steady_clock> m_end;
};

TraceLog::TraceLog(const std::string& functionName) : m_functionName(functionName), m_dataId(-1), m_isStop(false) {}

TraceLog::TraceLog(const std::string& functionName, uint32_t dataId) : m_functionName(functionName), m_dataId(dataId), m_isStop(false) {}

void TraceLog::start()
{
    m_start  = std::chrono::steady_clock::now();
    m_isStop = false;
}

void TraceLog::stop()
{
    if (m_isStop) return;
    m_isStop        = true;
    m_end           = std::chrono::steady_clock::now();
    auto elapsed_ms = vtf::utils::TimeUtil::convertTime<std::chrono::milliseconds>(m_end - m_start);
    if (m_dataId != -1)
    {
        VTF_LOGD("[func[{0}]:dataId[{1}]] execute {2} ms", m_functionName, m_dataId, elapsed_ms.count());
    }
    else
    {
        VTF_LOGD("[func[{0}]] execute {1} ms", m_functionName, elapsed_ms.count());
    }
}

TraceLog::~TraceLog()
{
    if (!m_isStop)
    {
        stop();
    }
}

}  // namespace log
}  // namespace utils
}  // namespace vtf