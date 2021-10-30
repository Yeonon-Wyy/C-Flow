/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 17:56:49
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-30 18:52:24
 */
#include "../core/pipeline/P1Node.hpp"
#include "../core/pipeline/P2Node.hpp"
#include "../core/pipeline/pipeRequest.hpp"
#include "../core/pipeline/pipenodeDispatcher.hpp"

int main()
{


    vtf::pipeline::PipeNodeDispatcher<vtf::pipeline::PipeRequest> pd(8);
    pd.run();

    while (true) {
        std::shared_ptr<vtf::pipeline::PipeRequest> req = std::make_shared<vtf::pipeline::PipeRequest>();
        pd.queueInDispacther(req);
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(300));
    }
    
}