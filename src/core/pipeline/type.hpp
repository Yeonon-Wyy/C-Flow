/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-11-10 20:52:24
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-09-04 19:20:13
 */
#pragma once

#include <condition_variable>
#include <mutex>

#include "../type.hpp"

namespace cflow::pipeline {
using PipelineScenario = uint32_t;
using PipeNodeId = cflow_id_t;

} // namespace cflow::pipeline