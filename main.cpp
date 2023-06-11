#include "utils/include/log.h"
#include "utils/include/id_generator.h"
#include "common/include/dag.h"
#include "utils/include/trace_log.h"
#include "common/include/task.h"
#include "common/include/dispatcher.h"
#include "common/include/notifier.h"
#include "common/include/scheduler.h"
#include "common/include/type.h"
#include "task/include/task_flow_ctl.h"
#include <iostream>




using namespace cflow;

int main()
{
    task::TaskFlowCtl tf(false);

    auto  add = [](int a, int b) {
        CFLOW_LOGD("{0} + {1}", a, b);
        return a + b;
    };

    auto minus = [](int a, int b) {
        std::this_thread::sleep_until(cflow::utils::TimeUtil::awake_time(30));
        CFLOW_LOGD("{0} - {1}", a, b);
        return a - b;
    };
    auto t0 = tf.addTask(add, 1, 2);
    auto t1 = tf.addTask(add, 1, 2);
    auto t2 = tf.addTask(minus, 3, 4);
    auto t3 = tf.addTask(add, 5, 6);
    auto t4 = tf.addTask(add, 7, 8);
    t0->connect(t1);
    t1->connect(t2);
    t1->connect(t3);
    t3->connect(t4);
    t2->connect(t4);
    tf.start();
}