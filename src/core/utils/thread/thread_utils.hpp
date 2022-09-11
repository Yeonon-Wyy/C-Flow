/*
 * @Author: Yeonon
 * @Date: 2022-09-11 20:41:56
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-09-11 20:43:26
 * @FilePath: /src/core/utils/thread/thread_utils.hpp
 * @Description: 
 * Copyright 2022 Yeonon, All Rights Reserved. 
 * 2022-09-11 20:41:56
 */
#pragma once

#include <pthread.h>
#include "../log/log.hpp"

namespace cflow::utils::thread
{
static void setScheduling(std::thread &th, int policy, int priority) {
    sched_param sch_params;
    sch_params.sched_priority = priority;
    if(pthread_setschedparam(th.native_handle(), policy, &sch_params)) {
        CFLOW_LOGE("Failed to set Thread scheduling : {0}", std::strerror(errno));
    }
}
} //namespace cflow::utils::thread