/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-09-22 21:53:09
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-12-25 19:08:42
 */
#include <cflow/core/task/tftask.hpp>
#include <cflow/core/utils/thread/threadPool.hpp>

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