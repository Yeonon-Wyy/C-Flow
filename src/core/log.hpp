/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-21 23:07:13
 * @LastEditors: yeonon
 * @LastEditTime: 2021-12-11 21:40:37
 */
#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

namespace vtf {

#ifndef LOGFILENAME
#define VTF_LOG_FILENAME LOGFILENAME
#else
#define VTF_LOG_FILENAME "/tmp/vtf_log/vtf_log.txt"
#endif
#define VTF_LOG_MAX_SIZE 1048576 * 5
#define VTF_LOG_MAX_FILE_SIZE 4
#define VTF_LOG_PATTERN "[%Y-%m-%d %H:%M:%S.%e][thread %t][%l]%v"

#define __FILENAME__ (strrchr(__FILE__,'/')?(strrchr(__FILE__,'/')+1):__FILE__)
#ifndef PREFIX
#define PREFIX(msg) std::string("").append("[")\
	.append(__FILENAME__).append(":").append(std::to_string(__LINE__)).append("]")\
    .append("[").append(__FUNCTION__).append("]")\
	.append(msg).c_str()
#endif //prefix

#define VTF_LOGT(msg,...)  vtf::Log::getInstance().getLogger()->trace(PREFIX(msg),## __VA_ARGS__)
#define VTF_LOGD(msg,...)  vtf::Log::getInstance().getLogger()->debug(PREFIX(msg),## __VA_ARGS__)
#define VTF_LOGI(msg,...)  vtf::Log::getInstance().getLogger()->info(PREFIX(msg),## __VA_ARGS__)
#define VTF_LOGW(msg,...) vtf::Log::getInstance().getLogger()->warn(PREFIX(msg),## __VA_ARGS__)
#define VTF_LOGE(msg,...)  vtf::Log::getInstance().getLogger()->error(PREFIX(msg),## __VA_ARGS__)
#define VTF_LOGC(msg,...)  vtf::Log::getInstance().getLogger()->critical(PREFIX(msg),## __VA_ARGS__)

class Log {
public:
    static Log& getInstance() 
    {
        static Log log;
        return log;
    }

    std::shared_ptr<spdlog::logger> getLogger() { return m_spdlogger; }

    ~Log() 
    {
    }
private:
    Log() 
    {
        #ifdef VTF_DEBUG_MODE
        //console log
        spdlog::stdout_color_mt("console");
        m_spdlogger = spdlog::get("console");

        m_spdlogger->set_level(spdlog::level::debug);
        m_spdlogger->set_pattern(VTF_LOG_PATTERN);
        m_spdlogger->flush_on(spdlog::level::err);
        #else
        m_spdlogger = spdlog::rotating_logger_mt("file", VTF_LOG_FILENAME, VTF_LOG_MAX_SIZE, VTF_LOG_MAX_FILE_SIZE);
        
        m_spdlogger->set_level(spdlog::level::debug);
        m_spdlogger->set_pattern(VTF_LOG_PATTERN);
        m_spdlogger->flush_on(spdlog::level::err);
        #endif
    }


private:
    std::shared_ptr<spdlog::logger> m_spdlogger;
    std::shared_ptr<spdlog::logger> m_consolelogger;
};
} //namespace vtf