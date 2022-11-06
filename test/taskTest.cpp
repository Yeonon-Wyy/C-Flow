/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-09-22 21:53:09
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-11-06 18:19:00
 */
#include "../src/core/task/tftask.hpp"
#include "../src/core/utils/thread/threadPool.hpp"

#include <iostream>
#include <functional>
#include <future>

int main()
{

    cflow::task::TFTask task;
    std::cout << task.ID() << std::endl;


    // cflow::ThreadPool pool(8);
    
}