/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2022-01-26 00:01:32
 * @LastEditors: yeonon
 * @LastEditTime: 2022-01-27 00:08:52
 */
#include "../src/core/scheduler.hpp"
#include "../src/core/pipeline/pipedata.hpp"



int main()
{
    
    vtf::Scheduler<std::shared_ptr<vtf::pipeline::PipeData>> scheduler;
    int cnt = 0;
    while (cnt < 33) {
        std::shared_ptr<vtf::pipeline::PipeData> data = std::make_shared<vtf::pipeline::PipeData>(0, false);
        scheduler.emplace(data);
    }

}