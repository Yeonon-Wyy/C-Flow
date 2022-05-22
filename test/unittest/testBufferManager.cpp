/*
 * @Author: Yeonon
 * @Date: 2022-05-21 20:08:02
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-05-22 18:31:22
 * @FilePath: /test/unittest/testBufferManager.cpp
 * @Description: 
 * Copyright 2022 Yeonon, All Rights Reserved. 
 * 2022-05-21 20:08:02
 */
#include "testUtils.hpp"
#include "../../src/core/utils/memory/buffer_manager.hpp"
#include <queue>


int main()
{
    vtf::BufferSpecification bfs = 
    {
        .sizeOfBytes = 16,
        .minQueueSize = 8,
        .maxQueueSize = 10,
    };

    vtf::BufferManager bfm(bfs);
    std::queue<std::shared_ptr<vtf::BufferInfo>> buffers;
    
    ASSERT_EQ(8, (int)bfm.availableCount());
    ASSERT_EQ(2, (int)bfm.noAlloctCount());

    
    for (int i = 0; i < 8; i++) {
        buffers.push(bfm.popBuffer());
        ASSERT_NOT_NULL(buffers.back()->ptr);
    };

    ASSERT_EQ(0, (int)bfm.availableCount());
    ASSERT_EQ(2, (int)bfm.noAlloctCount());

    
    buffers.push(bfm.popBuffer());
    ASSERT_NOT_NULL(buffers.back()->ptr);
    ASSERT_EQ(0, (int)bfm.availableCount());
    ASSERT_EQ(1, (int)bfm.noAlloctCount());


    buffers.push(bfm.popBuffer());
    ASSERT_NOT_NULL(buffers.back()->ptr);
    ASSERT_EQ(0, (int)bfm.availableCount());
    ASSERT_EQ(0, (int)bfm.noAlloctCount());


    bfm.pushBuffer(buffers.front());
    buffers.pop();
    ASSERT_EQ(1, (int)bfm.availableCount());
    ASSERT_EQ(0, (int)bfm.noAlloctCount());

    for (int i = 0; i < 7; i++) {
        bfm.pushBuffer(buffers.front());
        buffers.pop();
    }
    ASSERT_EQ(8, (int)bfm.availableCount());
    ASSERT_EQ(0, (int)bfm.noAlloctCount());


    bfm.pushBuffer(buffers.front());
    buffers.pop();
    ASSERT_EQ(8, (int)bfm.availableCount());
    ASSERT_EQ(1, (int)bfm.noAlloctCount());

    bfm.pushBuffer(buffers.front());
    buffers.pop();
    ASSERT_EQ(8, (int)bfm.availableCount());
    ASSERT_EQ(2, (int)bfm.noAlloctCount());


    std::cout << "now next stag." << std::endl;
    buffers.push(bfm.popBuffer());
    ASSERT_EQ(7, (int)bfm.availableCount());
    ASSERT_EQ(2, (int)bfm.noAlloctCount());
    ASSERT_NOT_NULL(buffers.back()->ptr);


    PRINT_RESULT();

    return 0;
}