/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-09-22 21:53:09
 * @LastEditors: yeonon
 * @LastEditTime: 2021-12-12 19:35:25
 */
#include "../src/core/task/task.hpp"
#include "../src/core/utils/thread/threadPool.hpp"

#include <iostream>
#include <functional>
#include <future>

int main()
{

    vtf::task::Task task1({1});
    auto task = task1.commit([](int a, int b) {
        return a + b;
    }, 1, 2);

    task1.setPriority(vtf::task::TaskPriority::URGENCY);
    
    vtf::task::Task task2({2});

    vtf::ThreadPool pool(4);
    auto future = pool.emplace([task]() {
        (*task)();
    });


    // vtf::ThreadPool pool(8);
    
}