/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-02 20:59:22
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-22 23:23:30
 */

#include "../core/threadPool.hpp"
#include "../core/task.hpp"
#include <iostream>


void testThreadPool()
{
    std::future<void> f;
    {
        std::packaged_task<void()> pt([]() { VTF_LOGI << "1"; });
        f = pt.get_future();
        pt();
        
    }
    f.get();
    
}

int main()
{
    testThreadPool();
}
