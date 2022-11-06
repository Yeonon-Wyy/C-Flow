/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-13 21:16:36
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-11-06 22:00:44
 */
#include "../src/core/task/task_flow_ctl.hpp"
#include "../src/core/utils/queue/blocking_queue.hpp"
#include "../src/core/utils/time_util.hpp"
#include "../src/core/type.hpp"
#include <chrono>

using namespace cflow::utils::memory;
using namespace cflow::task;

int main() {
    TaskFlowCtl tf(true);
    auto add = [](int a, int b) {
        std::cout << a << " + " << b << std::endl;
        return a + b;
    };

    auto minus = [](int a, int b) {
        std::this_thread::sleep_until(
            cflow::utils::TimeUtil::awake_time(30));
        std::cout << a << " - " << b << std::endl;
        return a - b;
    };
    auto t0 = tf.addTask(add, 1, 2);
    auto t1 = tf.addTask(add, 1, 2);
    auto t2 = tf.addTask(minus, 3, 4);
    auto t3 = tf.addTask(add, 5, 6);
    auto t4 = tf.addTask(add, 7, 8);
    t0->connect(t3);
    t1->connect(t2);
    t1->connect(t3);
    t3->connect(t4);
    t2->connect(t4);
    tf.start();
}