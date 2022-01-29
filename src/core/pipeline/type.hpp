/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-10 20:52:24
 * @LastEditors: yeonon
 * @LastEditTime: 2022-01-29 14:45:25
 */
#pragma once

#include <condition_variable>
#include <mutex>
#include "../type.hpp"

namespace vtf {
namespace pipeline {

using PipelineScenario = uint32_t;
using PipeNodeId = vtf_id_t;

}
}