/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-21 23:07:13
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-24 14:44:44
 */
#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

namespace vtf {

#define VTF_LOG_FILENAME "/tmp/vtf_log/vtf_log.txt"
#define VTF_LOG_MAX_SIZE 1048576 * 5
#define VTF_LOG_MAX_FILE_SIZE 4
#define VTF_LOG_PATTERN "[%Y-%m-%d %T][thread %t][%l]%v"

#define __FILENAME__ (strrchr(__FILE__,'/')?(strrchr(__FILE__,'/')+1):__FILE__)
#ifndef PREFIX
#define PREFIX(msg) std::string("").append("[")\
	.append(__FILENAME__).append(":").append(std::to_string(__LINE__)).append("]")\
    .append("[").append(__FUNCTION__).append("]")\
	.append(msg).c_str()
#endif //prefix

#ifdef VTF_DEBUG_MODE
#define VTF_LOGT(msg,...)  vtf::Log::getInstance().getConsoleLogger()->trace(PREFIX(msg),## __VA_ARGS__)
#define VTF_LOGD(msg,...)  vtf::Log::getInstance().getConsoleLogger()->debug(PREFIX(msg),## __VA_ARGS__)
#define VTF_LOGI(msg,...)  vtf::Log::getInstance().getConsoleLogger()->info(PREFIX(msg),## __VA_ARGS__)
#define VTF_LOGW(msg,...) vtf::Log::getInstance().getConsoleLogger()->warn(PREFIX(msg),## __VA_ARGS__)
#define VTF_LOGE(msg,...)  vtf::Log::getInstance().getConsoleLogger()->error(PREFIX(msg),## __VA_ARGS__)
#define VTF_LOGC(msg,...)  vtf::Log::getInstance().getConsoleLogger()->critical(PREFIX(msg),## __VA_ARGS__)
#else
#define VTF_LOGT(msg,...)  vtf::Log::getInstance().getLogger()->trace(PREFIX(msg),## __VA_ARGS__)
#define VTF_LOGD(msg,...)  vtf::Log::getInstance().getLogger()->debug(PREFIX(msg),## __VA_ARGS__)
#define VTF_LOGI(msg,...)  vtf::Log::getInstance().getLogger()->info(PREFIX(msg),## __VA_ARGS__)
#define VTF_LOGW(msg,...) vtf::Log::getInstance().getLogger()->warn(PREFIX(msg),## __VA_ARGS__)
#define VTF_LOGE(msg,...)  vtf::Log::getInstance().getLogger()->error(PREFIX(msg),## __VA_ARGS__)
#define VTF_LOGC(msg,...)  vtf::Log::getInstance().getLogger()->critical(PREFIX(msg),## __VA_ARGS__)
#endif
class Log {
public:
    static Log& getInstance() 
    {
        static Log log;
        return log;
    }

    std::shared_ptr<spdlog::logger> getLogger() { return m_spdlogger; }
    std::shared_ptr<spdlog::logger> getConsoleLogger() { return m_consolelogger; }

    ~Log() 
    {
    }
private:
    Log() 
    {
        m_spdlogger = spdlog::rotating_logger_mt("test", VTF_LOG_FILENAME, VTF_LOG_MAX_SIZE, VTF_LOG_MAX_FILE_SIZE);
        
        m_spdlogger->set_level(spdlog::level::debug);
        m_spdlogger->set_pattern(VTF_LOG_PATTERN);
        m_spdlogger->flush_on(spdlog::level::err);

        //console log
        spdlog::stdout_color_mt("console");
        m_consolelogger = spdlog::get("console");

        m_consolelogger->set_level(spdlog::level::debug);
        m_consolelogger->set_pattern(VTF_LOG_PATTERN);
        m_consolelogger->flush_on(spdlog::level::err);
    }


private:
    std::shared_ptr<spdlog::logger> m_spdlogger;
    std::shared_ptr<spdlog::logger> m_consolelogger;
};
} //namespace vtf