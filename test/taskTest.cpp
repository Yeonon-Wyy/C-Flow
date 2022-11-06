/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-09-22 21:53:09
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-11-06 20:51:56
 */
#include "../src/core/task/tftask.hpp"
#include "../src/core/utils/thread/threadPool.hpp"

#include <iostream>
#include <functional>
#include <future>

using namespace cflow::utils::memory;

int main()
{
    std::shared_ptr<BufferManagerFactory<void>> bufferMgrFactory =
        std::make_shared<BufferManagerFactory<void>>();
    cflow::task::TFTask task(bufferMgrFactory);
    task.setProcessFunc(
        [](int a, int b) {
            std::cout << a << " + " << b << std::endl;
            return a + b;
        },
        1, 2);
    task();
}