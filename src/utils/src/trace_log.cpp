#include "trace_log.h"

namespace cflow::utils::log {
TraceLog::TraceLog(const std::string& functionName)
    : m_functionName(functionName),
      m_dataId(-1),
      m_isStop(false)
{
}

TraceLog::TraceLog(const std::string& functionName, uint32_t dataId)
    : m_functionName(functionName),
      m_dataId(dataId),
      m_isStop(false)
{
}

void TraceLog::start()
{
    m_start  = std::chrono::steady_clock::now();
    m_isStop = false;
}

auto TraceLog::stop() -> std::chrono::milliseconds
{
    if (m_isStop) return std::chrono::milliseconds(0);
    m_isStop = true;
    m_end    = std::chrono::steady_clock::now();
    auto elapsed_ms =
        cflow::utils::TimeUtil::convertTime<std::chrono::milliseconds>(m_end -
                                                                       m_start);
    if (m_dataId != -1)
    {
        CFLOW_LOGD("[func[{0}]:dataId[{1}]] execute {2} ms", m_functionName,
                   m_dataId, elapsed_ms.count());
    }
    else
    {
        CFLOW_LOGD("[func[{0}]] execute {1} ms", m_functionName,
                   elapsed_ms.count());
    }
    return elapsed_ms;
}

TraceLog::~TraceLog()
{
    if (!m_isStop)
    {
        stop();
    }
}
} // namespace cflow::utils::log