/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-02 20:59:22
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-12-25 19:09:48
 */

#include <cflow/utils/threadPool.h>
#include <cflow/task/tftask.h>
#include <iostream>

void testThreadPool()
{
    std::future<void> f;
    {
        std::packaged_task<void()> pt([]() {});
        f = pt.get_future();
        pt();
    }
    f.get();
}

int main() { testThreadPool(); }
