
#include "../core/threadPool.hpp"
#include <iostream>

void testThreadPool()
{
    vtf::ThreadPool pool(4);

    auto featureRes = pool.emplace([](int a, int b){ return a + b; }, 1, 2);

    std::cout << featureRes.get() << std::endl;
    
}

int main()
{
    testThreadPool();
}
