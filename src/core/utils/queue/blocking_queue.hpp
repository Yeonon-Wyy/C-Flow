/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-16 22:00:26
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-07-17 17:24:45
 */
#pragma once

#include <condition_variable>
#include <mutex>
#include <vector>

#include "../log/log.hpp"

namespace vtf
{
namespace utils
{
namespace queue
{
template <typename T>
class BlockingQueue
{
   public:
    template <typename U>
    friend class BlockingQueue;

    explicit BlockingQueue(int capacity) noexcept : m_capacity(capacity), m_items(capacity + 1), m_startIdx(0), m_endIdx(0), m_stop(false) {}

    BlockingQueue() = delete;

    BlockingQueue(const BlockingQueue& rhs) noexcept : m_capacity(rhs.m_capacity), m_items(rhs.m_items), m_startIdx(rhs.m_startIdx), m_endIdx(rhs.m_endIdx) {}

    template <typename U>
    BlockingQueue(const BlockingQueue<U>& rhs) noexcept : m_capacity(rhs.m_capacity), m_items(rhs.m_items), m_startIdx(rhs.m_startIdx), m_endIdx(rhs.m_endIdx)
    {
    }

    BlockingQueue& operator=(BlockingQueue rhs) noexcept
    {
        swap(rhs);
        return *this;
    }

    template <typename U>
    BlockingQueue(BlockingQueue<U>&& rhs) noexcept
    {
        rhs.swap(*this);
        rhs.m_capacity = 0;
        rhs.m_items.reserve(0);
        rhs.m_startIdx = 0;
        rhs.m_endIdx   = 0;
    }

    void swap(BlockingQueue& rhs) noexcept
    {
        using std::swap;
        swap(m_capacity, rhs.m_capacity);
        swap(m_items, rhs.m_items);
        swap(m_startIdx, rhs.m_endIdx);
        swap(m_endIdx, rhs.m_endIdx);
    }

    void push(T item);
    T    pop();

    bool isEmpty() { return m_startIdx == m_endIdx; }
    bool isFull() { return (m_startIdx + m_capacity - m_endIdx) % (m_capacity + 1) == 0; }
    void clear();

    void stop();

   private:
    int            m_capacity;
    std::vector<T> m_items;
    int            m_startIdx;
    int            m_endIdx;
    bool           m_stop;

    std::mutex              m_mutex;
    std::condition_variable m_not_full;
    std::condition_variable m_not_empty;
};

template <typename T>
void BlockingQueue<T>::push(T item)
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (isFull())
        {
            m_not_full.wait(lock, [this]() { return !this->isFull() || m_stop; });
            if (m_stop) return;
        }

        m_items[m_endIdx++] = item;
        m_endIdx %= (m_capacity + 1);
    }

    m_not_empty.notify_one();
}

template <typename T>
T BlockingQueue<T>::pop()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (isEmpty())
    {
        m_not_empty.wait(lock, [this]() { return !this->isEmpty() || m_stop; });
        if (m_stop) return T();
    }
    T item = m_items[m_startIdx++];
    m_startIdx %= (m_capacity + 1);
    m_not_full.notify_one();
    return item;
}

template <typename T>
void BlockingQueue<T>::clear()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_startIdx = 0;
    m_endIdx   = 0;
    m_items.clear();
}

template <typename T>
void BlockingQueue<T>::stop()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stop = true;
    }
    m_not_full.notify_all();
    m_not_empty.notify_all();
}
}  // namespace queue
}  // namespace utils
}  // namespace vtf