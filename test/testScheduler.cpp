/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2022-01-26 00:01:32
 * @LastEditors: yeonon
 * @LastEditTime: 2022-01-27 00:08:52
 */
#include "../src/core/pipeline/pipedata.hpp"
#include "../src/core/scheduler.hpp"

int main() {

  cflow::Scheduler<std::shared_ptr<cflow::pipeline::PipeData>> scheduler;
  int cnt = 0;
  while (cnt < 33) {
    std::shared_ptr<cflow::pipeline::PipeData> data =
        std::make_shared<cflow::pipeline::PipeData>(0, false);
    scheduler.emplace(data);
  }
}