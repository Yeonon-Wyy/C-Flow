/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2022-01-26 00:01:32
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-10-11 22:23:59
 */
#include "../src/core/pipeline/pipe_task.hpp"
#include "../src/core/scheduler.hpp"

int main() {

  cflow::Scheduler<std::shared_ptr<cflow::pipeline::PipeTask>> scheduler;
  int cnt = 0;
  while (cnt < 33) {
    std::shared_ptr<cflow::pipeline::PipeTask> task =
        std::make_shared<cflow::pipeline::PipeTask>(0, false);
    scheduler.emplace(task);
  }
}