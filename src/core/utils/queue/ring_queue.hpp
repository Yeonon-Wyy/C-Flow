#pragma once

#include <vector>

namespace vtf {

template<typename E>
class RingQueue
{
public:
    RingQueue(int capcity)
        :m_capcity(capcity),
         m_elements(capcity+1),
         m_realIdx(0),
         m_frontIdx(0)
    {

    }

    bool push(E&& e)
    {
        if (full()) return false;
        m_elements[m_realIdx++] = e;
        m_realIdx %= (m_capcity+1);
        return true;
    }

    bool pop()
    {
        if (empty()) return false;
        m_frontIdx++;
        m_frontIdx %= (m_capcity+1);
        return true;
    }

    E real()
    {
        if (empty()) return E();
        int k = m_capcity+1;
        int realIdx = (((m_realIdx - 1) % k) + k) % k;
        return m_elements[realIdx];
    }

    E front()
    {
        if (empty()) return E();
        return m_elements[m_frontIdx];
    }

    bool empty()
    {
        return m_frontIdx == m_realIdx;
    }

    bool full()
    {
        return (m_realIdx + 1) % (m_capcity+1) == m_frontIdx;
    }

    std::size_t size() 
    {
        return (m_realIdx + (m_capcity + 1) - m_frontIdx) % (m_capcity + 1);
    }
private:
    int m_capcity;
    std::vector<E> m_elements;
    int m_realIdx;
    int m_frontIdx;
};

} //namespace vtf