/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-02 20:59:22
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-10 20:57:17
 */

#include "../core/threadPool.hpp"
#include "../core/task.hpp"
#include <iostream>


void testThreadPool()
{
    std::future<void> f;
    {
        std::packaged_task<void()> pt([]() { std::cout << "1" << std::endl; });
        f = pt.get_future();
        pt();
    }
    f.get();
    
}

int main()
{
    testThreadPool();
}
