/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-11-10 20:52:24
 * @LastEditors: yeonon
 * @LastEditTime: 2022-01-30 21:42:55
 */
#pragma once

#include <condition_variable>
#include <mutex>

#include "../type.hpp"

namespace cflow
{
namespace pipeline
{
using PipelineScenario = uint32_t;
using PipeNodeId       = cflow_id_t;

}  // namespace pipeline
}  // namespace cflow