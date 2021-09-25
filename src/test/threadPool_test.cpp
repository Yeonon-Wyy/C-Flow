
#include "../core/threadPool.hpp"
#include "../core/task.hpp"
#include <iostream>

void testThreadPool()
{
    vtf::ThreadPool pool(4);

    vtf::Task<int> task([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    auto featureRes = pool.emplace(task.getExecFunc());

    std::cout << featureRes.get() << std::endl;
    
}

int main()
{
    testThreadPool();
}
