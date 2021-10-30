/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-16 22:00:26
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-30 17:17:49
 */

#include <vector>
#include <mutex>
#include <condition_variable>

#include "log.hpp"

namespace vtf {
template <typename T>
class BlockingQueue
{
public:
    BlockingQueue(int capacity)
    :m_capacity(capacity),
     m_items(capacity+1),
     m_startIdx(0),
     m_endIdx(0)
    {}

    void push(T item);
    T pop();

    bool isEmpty() { return m_startIdx == m_endIdx; }
    bool isFull() { return (m_startIdx + m_capacity - m_endIdx) % (m_capacity + 1) == 0; }

private:
    int m_capacity;
    std::vector<T> m_items;
    int m_startIdx;
    int m_endIdx;

    std::mutex m_mutex;
    std::condition_variable m_not_full;
    std::condition_variable m_not_empty;
};

template<typename T>
void BlockingQueue<T>::push(T item)
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (isFull()) {
            m_not_full.wait(lock, [this]() {
                return !this->isFull();
            });
        }

        m_items[m_endIdx++] = item;
        m_endIdx %= (m_capacity + 1);
    }

    m_not_empty.notify_one();
}

template<typename T>
T BlockingQueue<T>::pop()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (isEmpty()) {
        m_not_empty.wait(lock, [this]() {
            return !this->isEmpty();
        });
    }
    T item = m_items[m_startIdx++];
    m_startIdx %= (m_capacity + 1);
    m_not_full.notify_one();
    return item;
}

} //namespace vtf