/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-02 20:59:22
 * @LastEditors: yeonon
 * @LastEditTime: 2021-12-12 19:36:08
 */

#include "../src/core/utils/thread/threadPool.hpp"
#include "../src/core/task/task.hpp"
#include <iostream>

void testThreadPool() {
  std::future<void> f;
  {
    std::packaged_task<void()> pt([]() {});
    f = pt.get_future();
    pt();
  }
  f.get();
}

int main() { testThreadPool(); }
