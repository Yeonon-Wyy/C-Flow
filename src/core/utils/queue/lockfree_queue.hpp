/*
 * @Author: Yeonon
 * @Date: 2022-07-24 16:13:47
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-08-06 17:22:37
 * @FilePath: /src/core/utils/queue/lockfree_queue.hpp
 * @Description:
 * Copyright 2022 Yeonon, All Rights Reserved.
 * 2022-07-24 16:13:47
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace vtf
{
namespace utils
{
namespace queue
{
/**
 * @description: Implementation of lock free queue.
 */
template <typename T>
class LockFreeQueue
{
   private:
    /**
     * @description:
     */
    struct Node
    {
        T     data;
        Node* next;
        Node(const T& _data) : data(_data), next(nullptr) {}
    };

   public:
    /**
     * @description: init head node and tail node.
     * @return {*}
     */
    LockFreeQueue();

    /**
     * @description: free all node
     * @return {*}
     */
    ~LockFreeQueue();

    /**
     * @description: push a data to queue
     * @return {*}
     */
    void push(T data);

    /**
     * @description: pop a data from queue
     * @param {T&} data, from queue
     * @return {*}
     */
    bool pop(T& data);

    /**
     * @description: if queue is empty, it will return true
     * @return {*}
     */
    bool empty() const { return m_head->next == nullptr; }

   private:
    Node* m_head;
    Node* m_tail;
};

template <typename T>
LockFreeQueue<T>::LockFreeQueue()
{
    Node* dummy = new Node(T());
    m_head      = dummy;
    m_tail      = dummy;
}

template <typename T>
LockFreeQueue<T>::~LockFreeQueue()
{
    while (m_head)
    {
        Node* tempHead = m_head;
        m_head         = m_head->next;
        delete tempHead;
    }
}

template <typename T>
void LockFreeQueue<T>::push(T data)
{
    Node* newNode = new Node(data);

    Node* p    = m_tail;
    Node* oldp = m_tail;

    // use GCC/G++ Build-in CAS function
    do
    {
        while (p->next != nullptr)
        {
            p = p->next;
        }
    } while (__sync_bool_compare_and_swap(&m_tail->next, nullptr, newNode) != true);
    __sync_bool_compare_and_swap(&m_tail, oldp, newNode);
}

template <typename T>
bool LockFreeQueue<T>::pop(T& data)
{
    // use GCC/G++ Build-in CAS function
    Node* p = nullptr;
    do
    {
        p = m_head;
        if (p->next == nullptr) return false;
    } while (__sync_bool_compare_and_swap(&m_head, p, p->next) != true);
    data = m_head->data;
    delete p;
    return true;
}

}  // namespace queue
}  // namespace utils
}  // namespace vtf