/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-21 23:07:13
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-09-03 19:05:53
 */
#pragma once

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>

namespace cflow
{
#ifdef LOGFILENAME
#define CFlow_LOG_FILENAME LOGFILENAME
#else
#define CFlow_LOG_FILENAME "/tmp/cflow_log/cflow_log.txt"
#endif
#define CFlow_LOG_MAX_SIZE 1048576 * 5
#define CFlow_LOG_MAX_FILE_SIZE 4
#define CFlow_LOG_PATTERN "[%Y-%m-%d %H:%M:%S.%e][thread %t][%l]%v"

#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)
#ifndef PREFIX
#define PREFIX(msg) std::string("").append("[").append(__FILENAME__).append(":").append(std::to_string(__LINE__)).append("]").append("[").append(__FUNCTION__).append("]").append(msg).c_str()
#endif  // prefix

#define CFLOW_LOGT(msg, ...) cflow::Log::getInstance().getLogger()->trace(PREFIX(msg), ##__VA_ARGS__)
#define CFLOW_LOGD(msg, ...) cflow::Log::getInstance().getLogger()->debug(PREFIX(msg), ##__VA_ARGS__)
#define CFLOW_LOGI(msg, ...) cflow::Log::getInstance().getLogger()->info(PREFIX(msg), ##__VA_ARGS__)
#define CFLOW_LOGW(msg, ...) cflow::Log::getInstance().getLogger()->warn(PREFIX(msg), ##__VA_ARGS__)
#define CFLOW_LOGE(msg, ...) cflow::Log::getInstance().getLogger()->error(PREFIX(msg), ##__VA_ARGS__)
#define CFLOW_LOGC(msg, ...) cflow::Log::getInstance().getLogger()->critical(PREFIX(msg), ##__VA_ARGS__)

class Log
{
public:
    static Log& getInstance()
    {
        static Log log;
        return log;
    }

    std::shared_ptr<spdlog::logger> getLogger() { return m_spdlogger; }

    ~Log() {}

private:
    Log()
    {
#ifdef CFlow_DEBUG_MODE
        // console log
        spdlog::stdout_color_mt("console");
        m_spdlogger = spdlog::get("console");

        m_spdlogger->set_level(spdlog::level::debug);
        m_spdlogger->set_pattern(CFlow_LOG_PATTERN);
        m_spdlogger->flush_on(spdlog::level::err);
#else
        m_spdlogger = spdlog::rotating_logger_mt("file", CFlow_LOG_FILENAME, CFlow_LOG_MAX_SIZE, CFlow_LOG_MAX_FILE_SIZE);

        m_spdlogger->set_level(spdlog::level::debug);
        m_spdlogger->set_pattern(CFlow_LOG_PATTERN);
        m_spdlogger->flush_on(spdlog::level::err);
#endif
    }

private:
    std::shared_ptr<spdlog::logger> m_spdlogger;
    std::shared_ptr<spdlog::logger> m_consolelogger;
};
}  // namespace cflow