/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2022-01-23 19:32:20
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-11-06 21:33:26
 */
#pragma once
#include <memory>
#include <type_traits>

#define RT_TASK_CAPCITY 8
#define NORMAL_TASK_CAPCITY 16
#define IDEL_TASK_CAPCITY 32

#define UNUSED(x) (void)(x)

namespace cflow {
using cflow_id_t = long;

enum class NotifierType
{
    START,
    TASK_LISTEN,
    FINAL,
    END
};

enum class NotifyStatus
{
    OK,
    ERROR
};

enum class TaskType
{
    START = 0,
    RT,
    NORMAL,
    IDEL,
    END
};

enum class TaskPriority
{
    START = 0,
    URGENCY,
    NORMAL,
    IDEL,
    END
};

enum class TaskStatus
{
    OK    = 0,
    ERROR = 1,
};

// utils
template <typename T>
struct is_shared_ptr
{
    static const bool value = false;
};

template <typename T>
struct is_shared_ptr<std::shared_ptr<T>>
{
    static const bool value = true;
};

} // namespace cflow