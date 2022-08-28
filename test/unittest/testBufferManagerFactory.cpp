/*
 * @Author: Yeonon
 * @Date: 2022-05-29 15:54:57
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-06-25 19:24:11
 * @FilePath: /test/unittest/testBufferManagerFactory.cpp
 * @Description:
 * Copyright 2022 Yeonon, All Rights Reserved.
 * 2022-05-29 15:54:57
 */

#include "../../src/core/utils/memory/buffer_manager.hpp"
#include "../../src/core/utils/memory/buffer_manager_factory.hpp"

#include "testUtils.hpp"

using namespace vtf::utils::memory;

void testBufferMnaagerInterface() {
  BufferManagerFactory<int> bmf;

  BufferSpecification bfs1 = {
      .sizeOfBytes = 0, .minQueueSize = 8, .maxQueueSize = 10, .name = "bfs1"};

  BufferSpecification bfs2 = {
      .sizeOfBytes = 0, .minQueueSize = 8, .maxQueueSize = 10, .name = "bfs2"};

  BufferSpecification bfs3 = {
      .sizeOfBytes = 0, .minQueueSize = 8, .maxQueueSize = 10, .name = "bfs3"};

  BufferSpecification bfs4 = {
      .sizeOfBytes = 0, .minQueueSize = 8, .maxQueueSize = 10, .name = "bfs4"};

  auto bm1 = bmf.createBufferManager(bfs1);
  auto bm2 = bmf.createBufferManager(bfs2);
  auto bm3 = bmf.createBufferManager(bfs3);

  ASSERT_EQ((int)bm1->availableCount(), 8);
  ASSERT_EQ((int)bm2->availableCount(), 8);
  ASSERT_EQ((int)bm3->availableCount(), 8);

  ASSERT_EQ(bmf.getBufferManager(bfs1)->name(), bm1->name());
  ASSERT_EQ(bmf.getBufferManager(bfs2)->name(), bm2->name());
  ASSERT_EQ(bmf.getBufferManager(bfs3)->name(), bm3->name());

  auto bm5 = bmf.getBufferManager(bfs4);
  ASSERT_NULL(bm5);

  bmf.releaseBufferManager(bfs1);

  ASSERT_NULL(bmf.getBufferManager(bfs1));

  PRINT_RESULT();
}

void testEmptyBufferManager() {
  BufferManagerFactory<int> bmf;
  BufferSpecification bfs1 = {
      .sizeOfBytes = 0, .minQueueSize = 8, .maxQueueSize = 10, .name = "bfs1"};
  auto bm1 = bmf.createBufferManager(bfs1);
}

int main() {
  testBufferMnaagerInterface();
  testEmptyBufferManager();
}