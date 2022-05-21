/*
 * @Author: Yeonon
 * @Date: 2022-05-21 17:00:57
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-05-21 20:52:15
 * @FilePath: /src/core/utils/queue/ring_queue.hpp
 * @Description: 
 * Copyright 2022 Yeonon, All Rights Reserved. 
 * 2022-05-21 17:00:57
 */
#pragma once

#include <vector>

namespace vtf {


/**
 * @description: RingQueue is a ring queue, receive a fix size, and memory can reuse.
 * @return {*}
 */
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

    /**
     * @description: push element to queue
     * @param {E&&} e
     * @return {*}
     */    
    bool push(E&& e)
    {
        if (full()) return false;
        m_elements[m_realIdx++] = e;
        m_realIdx %= (m_capcity+1);
        return true;
    }

    /**
     * @description: pop a element
     * @return {*}
     */    
    bool pop()
    {
        if (empty()) return false;
        m_frontIdx++;
        m_frontIdx %= (m_capcity+1);
        return true;
    }

    /**
     * @description: get real element
     * @return {*}
     */    
    E real()
    {
        if (empty()) return E();
        int k = m_capcity+1;
        int realIdx = (((m_realIdx - 1) % k) + k) % k;
        return m_elements[realIdx];
    }

    /**
     * @description: get front element
     * @return {*}
     */
    E front()
    {
        if (empty()) return E();
        return m_elements[m_frontIdx];
    }

    /**
     * @description: is empty
     * @return {*}
     */    
    bool empty()
    {
        return m_frontIdx == m_realIdx;
    }

    /**
     * @description: is full
     * @return {*}
     */
    bool full()
    {
        return (m_realIdx + 1) % (m_capcity+1) == m_frontIdx;
    }

    /**
     * @description: return size of queue
     * @return {*}
     */
    std::size_t size() 
    {
        return (m_realIdx + (m_capcity + 1) - m_frontIdx) % (m_capcity + 1);
    }

    /**
     * @description: return capcity of queue
     * @return {*}
     */
    std::size_t capcity() { return m_capcity; }
private:
    int m_capcity;
    std::vector<E> m_elements;
    int m_realIdx;
    int m_frontIdx;
};

} //namespace vtf