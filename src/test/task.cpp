/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-09-22 21:53:09
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-02 17:18:28
 */
#include "../core/task.hpp"
#include "../core/threadPool.hpp"

#include <iostream>
#include <functional>
#include <future>

int main()
{
    vtf::Task<int> task1([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    task1.setPriority(vtf::TaskPriority::URGENCY);
    std::cout << task1.getPriority() << std::endl;

    task1.getExecFunc()();

    
}