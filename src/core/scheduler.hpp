/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2022-01-22 20:06:27
 * @LastEditors: yeonon
 * @LastEditTime: 2022-02-06 17:11:56
 */
#pragma once

#include <queue>
#include <unordered_map>
#include <map>
#include <cstdlib>
#include "log.hpp"
#include "type.hpp"

namespace vtf {



template<typename T>
class Scheduler {
public:

    struct ItemPriorityComp {
        template<typename Q = T>
        typename std::enable_if<std::is_pointer<Q>::value || vtf::is_shared_ptr<Q>::value, bool>::type operator() (T lhs, T rhs)
        {
            return lhs->getPriority() < rhs->getPriority();
        }

        template<typename Q = T>
        typename std::enable_if<!std::is_pointer<Q>::value && !vtf::is_shared_ptr<Q>::value, bool>::type operator() (T lhs, T rhs)
        {
            return lhs.getPriority() < rhs.getPriority();;
        }
    };

    using SchedulerQueue = std::priority_queue<T, std::vector<T>, ItemPriorityComp>;
public:
    Scheduler();

    void emplace(T item);
    T schedule();

    bool empty();
    size_t getQueueCapWithFromItem(T item);
    size_t getQueueSizeWithFromItem(T item);


private:

    template<typename Q = T>
    typename std::enable_if<std::is_pointer<Q>::value || vtf::is_shared_ptr<Q>::value, DataType>::type extractDataTypeFromItem(T item)
    {
        return item->getDataType();
    }

    template<typename Q = T>
    typename std::enable_if<!std::is_pointer<Q>::value && !vtf::is_shared_ptr<Q>::value, DataType>::type extractDataTypeFromItem(T item)
    {
        return item.getDataType();
    }

private:
    std::map<DataType, SchedulerQueue> m_dataTypeQueueMap;
    std::unordered_map<DataType, size_t> m_dataTypeQueueCapMap;
};


// ---------------- Scheduler begin----------------

template<typename T>
Scheduler<T>::Scheduler()
{
    m_dataTypeQueueMap[DataType::DATATYPE_RT] = SchedulerQueue();
    m_dataTypeQueueCapMap[DataType::DATATYPE_RT] = 8;
    m_dataTypeQueueMap[DataType::DATATYPE_NORMAL] = SchedulerQueue();
    m_dataTypeQueueCapMap[DataType::DATATYPE_NORMAL] = 8;
    m_dataTypeQueueMap[DataType::DATATYPE_IDEL] = SchedulerQueue();
    m_dataTypeQueueCapMap[DataType::DATATYPE_IDEL] = 8;
}

template<typename T>
void Scheduler<T>::emplace(T item)
{
    auto curItemDataType = extractDataTypeFromItem(item);
    if (curItemDataType <= DataType::DATATYPE_START || curItemDataType >= DataType::DATATYPE_END) {
        VTF_LOGE("please check current data's data type (%d)", curItemDataType);
        std::abort();
    }
    m_dataTypeQueueMap[curItemDataType].push(item);
}

template<typename T>
T Scheduler<T>::schedule()
{
    T item;

    for (auto&[dataType, dataQueue] : m_dataTypeQueueMap) {
        if (dataQueue.empty()) continue;
        item = dataQueue.top();
        dataQueue.pop();
        break;
    }
    return item;
}

template<typename T>
bool Scheduler<T>::empty()
{
    size_t curSz = 0;
    for (auto&[dataType, itemQueue] : m_dataTypeQueueMap) {
        curSz += itemQueue.size();
    } 
    return curSz == 0;
}

template<typename T>
size_t Scheduler<T>::getQueueCapWithFromItem(T item)
{
    auto curItemDataType = extractDataTypeFromItem(item);

    if (curItemDataType <= DataType::DATATYPE_START 
        || curItemDataType >= DataType::DATATYPE_END
        || m_dataTypeQueueCapMap.count(curItemDataType) == 0) {
        VTF_LOGE("please check current data's data type (%d)", curItemDataType);
        std::abort();
    }
    return m_dataTypeQueueCapMap[curItemDataType];
}

template<typename T>
size_t Scheduler<T>::getQueueSizeWithFromItem(T item)
{
    auto curItemDataType = extractDataTypeFromItem(item);

    if (curItemDataType <= DataType::DATATYPE_START 
        || curItemDataType >= DataType::DATATYPE_END
        || m_dataTypeQueueMap.count(curItemDataType) == 0) {
        VTF_LOGE("please check current data's data type (%d)", curItemDataType);
        std::abort();
    }
    return m_dataTypeQueueMap[curItemDataType].size();
}
// ---------------- Scheduler end----------------

} //namespace vtf