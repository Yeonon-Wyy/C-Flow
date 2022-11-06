/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-09-22 21:53:09
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-11-06 20:01:20
 */
#include "../src/core/task/tftask.hpp"
#include "../src/core/utils/thread/threadPool.hpp"

#include <iostream>
#include <functional>
#include <future>

int main()
{

    cflow::task::TFTask task;
    task.setProcessFunc([](int a, int b) { std::cout << a << " + " << b << std::endl; return a + b; }, 1, 2);
    task();
    
}