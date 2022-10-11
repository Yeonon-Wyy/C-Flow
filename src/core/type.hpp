/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2022-01-23 19:32:20
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-10-11 23:01:54
 */
#pragma once
#include <memory>
#include <type_traits>

#define RT_TASK_CAPCITY 8
#define NORMAL_TASK_CAPCITY 16
#define IDEL_TASK_CAPCITY 32


namespace cflow {
using cflow_id_t = long;

enum class NotifierType
{
    NOTIFIER_TYPE_START,
    TASK_LISTEN,
    FINAL,
    NOTIFIER_TYPE_END
};

enum class NotifyStatus
{
    OK,
    ERROR
};

enum class TaskType
{
    TASKTYPE_START = 0,
    TASKTYPE_RT,
    TASKTYPE_NORMAL,
    TASKTYPE_IDEL,
    TASKTYPE_END
};

enum class TaskPriority
{
    TASKPRIORITY_START = 0,
    TASKPRIORITY_URGENT,
    TASKPRIORITY_NORMAL,
    TASKPRIORITY_IDEL,
    TASKPRIORITY_END
};

enum class TaskStatus 
{
    OK     = 0,
    ERROR  = 1,
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

template <typename T>
class is_default_constructible
{
    typedef char yes;
    typedef struct
    {
        char arr[2];
    } no;

    template <typename U>
    static decltype(U(), yes()) test(int);

    template <typename U>
    no test(...);

public:
    static const bool value = sizeof(test<T>(0)) == sizeof(yes);
};

} // namespace cflow