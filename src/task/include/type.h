/*
 * @Author: Yeonon
 * @Date: 2022-11-06 18:07:58
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-11-06 19:39:19
 * @FilePath: /src/core/task/type.h
 * @Description:
 * Copyright 2022 Yeonon, All Rights Reserved.
 * 2022-11-06 18:07:58
 */
#pragma once
#include <cflow/common/type.h>
#include <cflow/utils/buffer_manager.h>
#include <cflow/utils/buffer_manager_factory.h>

#include <functional>

namespace cflow::task {
using namespace cflow::utils::memory;

using TFTaskScenario     = uint32_t;
using TFTaskFunc         = std::function<void()>;
using TFTaskBufferInfoSP = std::shared_ptr<BufferManager<void>::BufferInfo>;

struct TaskCreateInfo
{
    TaskType            taskType = cflow::TaskType::NORMAL;
    cflow::TaskPriority priority = cflow::TaskPriority::NORMAL;
    std::string         name     = "";
    TFTaskFunc          processFunc;
};

struct NodeBufferInfo
{
    cflow_id_t                      nodeId;
    std::vector<TFTaskBufferInfoSP> input;
    std::vector<TFTaskBufferInfoSP> output;
};

using NodeBufferInfoSP = std::shared_ptr<NodeBufferInfo>;
} // namespace cflow::task