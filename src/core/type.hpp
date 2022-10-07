/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2022-01-23 19:32:20
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-10-07 19:36:30
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
    DATA_LISTEN,
    FINAL,
    NOTIFIER_TYPE_END
};

enum class NotifyStatus
{
    OK,
    ERROR
};

enum class DataType
{
    DATATYPE_START = 0,
    DATATYPE_RT,
    DATATYPE_NORMAL,
    DATATYPE_IDEL,
    DATATYPE_END
};

enum class DataPriority
{
    DATAPRIORITY_START = 0,
    DATAPRIORITY_URGENT,
    DATAPRIORITY_NORMAL,
    DATAPRIORITY_IDEL,
    DATAPRIORITY_END
};

enum class DataStatus 
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