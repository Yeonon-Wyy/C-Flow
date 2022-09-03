/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-09-22 21:53:09
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-07-24 16:44:41
 */
#include "../src/core/task/task.hpp"
#include "../src/core/utils/thread/threadPool.hpp"

#include <iostream>
#include <functional>
#include <future>

int main()
{

    cflow::task::Task task1({1});
    auto task = task1.commit([](int a, int b) {
        return a + b;
    }, 1, 2);

    task1.setPriority(cflow::task::TaskPriority::URGENCY);
    
    cflow::task::Task task2({2});

    cflow::utils::thread::ThreadPool pool(4);
    auto future = pool.emplace([task]() {
        (*task)();
    });


    // cflow::ThreadPool pool(8);
    
}