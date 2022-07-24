/*
 * @Author: Yeonon
 * @Date: 2022-07-24 16:13:47
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-07-24 16:36:21
 * @FilePath: /src/core/utils/queue/lockfree_queue.hpp
 * @Description: 
 * Copyright 2022 Yeonon, All Rights Reserved. 
 * 2022-07-24 16:13:47
 */

#pragma once

#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <atomic>

namespace vtf {
namespace utils {
namespace queue {

template<typename T>
class LockFreeQueue
{
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& _data)
            :data(_data),
             next(nullptr)
        {
            
        }
    };

public:
    LockFreeQueue() 
    {
        Node* dummy = new Node(T());
        m_head = dummy;
        m_tail = dummy;
    }

    ~LockFreeQueue() 
    {
        while (m_head) {
            Node* tempHead = m_head;
            m_head = m_head->next;
            delete tempHead;
        }
    }

    void push(T data);
    bool pop(T& data);

    bool empty() { return m_head->next == nullptr; }

private:
    Node* m_head;
    Node* m_tail;
};

template<typename T>
void LockFreeQueue<T>::push(T data)
{
    Node* newNode = new Node(data);

    Node* p = m_tail;
    Node* oldp = m_tail;

    do {
        while (p->next != nullptr) {
            p = p->next;
        }
    } while (__sync_bool_compare_and_swap(&m_tail->next, nullptr, newNode) != true);
    __sync_bool_compare_and_swap(&m_tail, oldp, newNode);
}

template<typename T>
bool LockFreeQueue<T>::pop(T& data)
{
    Node* p = nullptr;
    do {
        p = m_head;
        if (p->next == nullptr) return false;
    } while (__sync_bool_compare_and_swap(&m_head, p, p->next) != true);
    data = m_head->data;
    delete p;
    return true;
}

} //namespace queue
} //namesapce util
} //namespace vtf