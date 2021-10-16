/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-09-22 21:53:09
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-16 18:06:46
 */
#include "../core/task.hpp"
#include "../core/threadPool.hpp"

#include <iostream>
#include <functional>
#include <future>

int main()
{

    vtf::Task task1({1});
    auto task = task1.commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    task1.setPriority(vtf::TaskPriority::URGENCY);
    std::cout << task1.getPriority() << std::endl;
    
    vtf::Task task2();

    vtf::ThreadPool pool(4);
    auto future = pool.emplace([task]() {
        (*task)();
    });

    std::cout << task->get_future().get() << std::endl;

    // vtf::ThreadPool pool(8);
    
}