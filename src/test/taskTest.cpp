/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-09-22 21:53:09
 * @LastEditors: yeonon
 * @LastEditTime: 2021-12-03 23:19:21
 */
#include "../core/task/task.hpp"
#include "../core/threadPool.hpp"

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