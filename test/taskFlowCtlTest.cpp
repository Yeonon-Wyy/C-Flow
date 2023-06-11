/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-13 21:16:36
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-12-25 19:08:30
 */
#include <cflow/task/task_flow_ctl.h>
#include <cflow/utils/blocking_queue.h>
#include <cflow/utils/time_util.h>
#include <cflow/common/type.h>
#include <chrono>

using namespace cflow::utils::memory;
using namespace cflow::task;

int main()
{
    TaskFlowCtl tf(true);
    auto        add = [](int a, int b) {
        std::cout << a << " + " << b << std::endl;
        return a + b;
    };

    auto minus = [](int a, int b) {
        std::this_thread::sleep_until(cflow::utils::TimeUtil::awake_time(30));
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