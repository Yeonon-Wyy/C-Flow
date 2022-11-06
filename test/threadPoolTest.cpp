/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-02 20:59:22
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-11-06 18:21:09
 */

#include "../src/core/utils/thread/threadPool.hpp"
#include "../src/core/task/tftask.hpp"
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
