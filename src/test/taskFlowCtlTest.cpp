/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-13 21:16:36
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-13 22:08:17
 */
#include "../core/taskflowctl.hpp"
#include "../core/utils.hpp"
#include <chrono>

int main()
{
    vtf::TaskFlowCtl flowCtl(true);
    auto task1 = flowCtl.addTask([]() {
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
    });

    auto task2 = flowCtl.addTask([](int a, int b) {
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
    }, 1, 2);

    auto task3 = flowCtl.addTask([](int a, int b) {
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
    }, 1, 2);

    auto task4 = flowCtl.addTask([](int a, int b) {
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
    }, 1, 2);

    auto task5 = flowCtl.addTask([](int a, int b) {
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(2200));
    }, 1, 2);

    auto task6 = flowCtl.addTask([](int a, int b) {
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
    }, 1, 2);

    auto task7 = flowCtl.addTask([](int a, int b) {
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
    }, 1, 2);
    

    task4->precede(task3);
    task4->precede(task6);
    task3->precede(task6);
    task3->precede(task2);
    task6->precede(task5);
    task6->precede(task1);
    task1->precede(task2);
    task5->precede(task7);  
    task1->precede(task7);
    // task2->precede(task7);

    auto start = std::chrono::system_clock::now();
    flowCtl.start();
    auto end = std::chrono::system_clock::now();
    std::cout << "all task need: " << vtf::util::TimeUtil::convertTime<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

}