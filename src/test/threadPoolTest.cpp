/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-02 20:59:22
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-24 13:43:31
 */

#include "../core/threadPool.hpp"
#include "../core/task.hpp"
#include <iostream>


void testThreadPool()
{
    std::future<void> f;
    {
        std::packaged_task<void()> pt([]() { });
        f = pt.get_future();
        pt();
        
    }
    f.get();
    
}

int main()
{
    testThreadPool();
}
