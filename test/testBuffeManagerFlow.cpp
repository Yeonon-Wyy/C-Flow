/*
 * @Author: Yeonon
 * @Date: 2022-05-28 15:48:33
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-05-28 16:14:26
 * @FilePath: /test/testBuffeManagerFlow.cpp
 * @Description: 
 * Copyright 2022 Yeonon, All Rights Reserved. 
 * 2022-05-28 15:48:33
 */
#include "../src/core/utils/memory/buffer_manager.hpp"
#include "../src/core/utils/queue/blocking_queue.hpp"
#include <thread>
#include <queue>
#include <mutex>


std::mutex g_lock;
std::condition_variable g_cv;

using BufferInfo = std::shared_ptr<vtf::BufferManager<int>::BufferInfo>;

void getBuffer(vtf::BufferManager<int>& bmgr, vtf::BlockingQueue<BufferInfo>& curBufQ)
{
    for (int i = 0; i < 100; i++) {
        BufferInfo bf = bmgr.popBuffer();
        curBufQ.push(bf);
        VTF_LOGD("get buffer : {0} curBufQ.isempty : {1}", (void*)bf->ptr, curBufQ.isEmpty());
    }
}

void returnBuffer(vtf::BufferManager<int>& bmgr, vtf::BlockingQueue<BufferInfo>& curBufQ)
{
    for (int i = 0; i < 100; i++) {
        auto bf = curBufQ.pop();
        bmgr.pushBuffer(bf);
        VTF_LOGD("return buffer : {0}", (void*)bf->ptr);
    }
}

int main()
{
    vtf::BufferSpecification bfs = 
    {
        .sizeOfBytes = 0,
        .minQueueSize = 8,
        .maxQueueSize = 10,
    };
    
    vtf::BlockingQueue<BufferInfo> curBufQ(10);
    vtf::BufferManager<int> bmgr(bfs);
    std::thread t1(getBuffer, std::ref(bmgr), std::ref(curBufQ));
    std::thread t2(returnBuffer, std::ref(bmgr), std::ref(curBufQ));


    t1.join();
    t2.join();
}