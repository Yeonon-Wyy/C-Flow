/*
 * @Author: Yeonon
 * @Date: 2022-09-11 20:41:56
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-10-07 16:30:50
 * @FilePath: /src/core/utils/thread/thread_utils.hpp
 * @Description:
 * Copyright 2022 Yeonon, All Rights Reserved.
 * 2022-09-11 20:41:56
 */
#pragma once

#include <pthread.h>
#include "../log/log.hpp"

namespace cflow::utils::thread {
#ifdef __linux__

/**
 * @description: 
 * @param {thread} &th    thread object
 * @param {int} policy    thread policy, like RR,FIFO,etc....
 * @param {int} priority  number of priority
 * @return {*}
 */
static inline void setScheduling(std::thread &th, int policy, int priority)
{
    sched_param sch_params;
    sch_params.sched_priority = priority;
    if (pthread_setschedparam(th.native_handle(), policy, &sch_params))
    {
        CFLOW_LOGE("Failed to set Thread scheduling : {0}",
                   std::strerror(errno));
    }
}
#else
static inline void setScheduling(std::thread &th, int policy, int priority) { return; }
#endif
} // namespace cflow::utils::thread