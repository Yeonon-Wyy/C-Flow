/*
 * @Author: Yeonon
 * @Date: 2022-05-28 15:48:33
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-12-25 19:09:04
 * @FilePath: /test/testBuffeManagerFlow.cpp
 * @Description:
 * Copyright 2022 Yeonon, All Rights Reserved.
 * 2022-05-28 15:48:33
 */
#include <cflow/utils/buffer_manager.h>
#include <cflow/utils/blocking_queue.h>
#include <mutex>
#include <queue>
#include <thread>

std::mutex              g_lock;
std::condition_variable g_cv;
using namespace cflow::utils::memory;
using namespace cflow::utils::queue;

using BufferInfo = std::shared_ptr<BufferManager<int>::BufferInfo>;

void getBuffer(BufferManager<int> &bmgr, BlockingQueue<BufferInfo> &curBufQ)
{
    for (int i = 0; i < 100; i++)
    {
        BufferInfo bf = bmgr.popBuffer();
        curBufQ.push(bf);
        CFLOW_LOGD("get buffer : {0} curBufQ.isempty : {1}", (void *)bf->ptr,
                   curBufQ.isEmpty());
    }
}

void returnBuffer(BufferManager<int> &bmgr, BlockingQueue<BufferInfo> &curBufQ)
{
    for (int i = 0; i < 100; i++)
    {
        auto bf = curBufQ.pop();
        bmgr.pushBuffer(bf);
        CFLOW_LOGD("return buffer : {0}", (void *)bf->ptr);
    }
}

int main()
{
    BufferSpecification bfs = {
        .sizeOfBytes  = 0,
        .minQueueSize = 8,
        .maxQueueSize = 10,
    };

    BlockingQueue<BufferInfo> curBufQ(10);
    BufferManager<int>        bmgr(bfs);
    std::thread               t1(getBuffer, std::ref(bmgr), std::ref(curBufQ));
    std::thread t2(returnBuffer, std::ref(bmgr), std::ref(curBufQ));

    t1.join();
    t2.join();
}