/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-20 21:12:25
 * @LastEditors: yeonon
 * @LastEditTime: 2021-12-12 19:34:23
 */
#include "../src/core/utils/queue/blocking_queue.hpp"
#include <random>
#include <iostream>
#include <thread>


void produce(vtf::BlockingQueue<int>& bq)
{
    for (int i = 0; i < 100; i++) {
        bq.push(i);
    }
}

void consume(vtf::BlockingQueue<int>& bq, int i)
{
    while (true) {
        std::random_device rd;
        auto               item = bq.pop();
        VTF_LOGI("consume {0} [{1}]", i, item);
        std::this_thread::sleep_for(std::chrono::milliseconds(1500 + rd() % 5000));
    }
}

int main(int agrc, char** agrv)
{

    vtf::BlockingQueue<int> bq(8);
    std::thread t1(consume, std::ref(bq), 1);
    std::thread t2(consume, std::ref(bq), 2);
    std::thread t3(consume, std::ref(bq), 3);
    std::thread t4(consume, std::ref(bq), 4);

    std::thread t5(produce, std::ref(bq));


    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
}