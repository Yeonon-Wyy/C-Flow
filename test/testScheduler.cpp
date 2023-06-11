/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2022-01-26 00:01:32
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-12-25 19:09:22
 */
#include <cflow/pipeline/pipe_task.h>
#include <cflow/common/scheduler.h>

int main()
{
    cflow::Scheduler<std::shared_ptr<cflow::pipeline::PipeTask>> scheduler;
    int                                                          cnt = 0;
    while (cnt < 33)
    {
        std::shared_ptr<cflow::pipeline::PipeTask> task =
            std::make_shared<cflow::pipeline::PipeTask>(0, false);
        scheduler.emplace(task);
        cnt++;
    }
}