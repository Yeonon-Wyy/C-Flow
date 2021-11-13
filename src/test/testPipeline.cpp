/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 17:56:49
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-13 00:11:40
 */
#include "../core/pipeline/pipeRequest.hpp"
#include "../core/pipeline/pipenodeDispatcher.hpp"
#include "../core/pipeline/pipeline.hpp"

using namespace vtf::pipeline;

void testDispatch()
{
    vtf::pipeline::PipeNodeDispatcher<vtf::pipeline::PipeRequest> pd(8);
    pd.run();

    while (true) {
        std::shared_ptr<vtf::pipeline::PipeRequest> req = std::make_shared<vtf::pipeline::PipeRequest>(PipelineScenario::SAT);
        pd.queueInDispacther(req);
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(300));
    }
}

void testPipeline()
{
    vtf::pipeline::PipeLine<vtf::pipeline::Request> ppl;
    auto node1 = ppl.addPipeNode([](std::shared_ptr<vtf::pipeline::Request>) -> bool {
        VTF_LOGD("ppl add node 1");
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(20));
        return true;
    });

    auto node2 = ppl.addPipeNode([](std::shared_ptr<vtf::pipeline::Request>) -> bool {
        VTF_LOGD("ppl add node 2");
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(20));
        return true;
    });

    auto node3 = ppl.addPipeNode([](std::shared_ptr<vtf::pipeline::Request>) -> bool {
        VTF_LOGD("ppl add node 3");
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(20));
        return true;
    });

    auto node4 = ppl.addPipeNode([](std::shared_ptr<vtf::pipeline::Request>) -> bool {
        VTF_LOGD("ppl add node 4");
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(20));
        return true;
    });

    auto node5 = ppl.addPipeNode([](std::shared_ptr<vtf::pipeline::Request>) -> bool {
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(20));
        VTF_LOGD("ppl add node 4");
        return true;
    });

    //[[1],[2,3],[4]]
    node1->addcenarios(PipelineScenario::SAT)->addcenarios(PipelineScenario::BOKEH);
    node2->addcenarios(PipelineScenario::SAT);
    node3->addcenarios(PipelineScenario::BOKEH);
    node4->addcenarios(PipelineScenario::SAT)->addcenarios(PipelineScenario::BOKEH);

    node1->connect(node2);
    node1->connect(node3);
    node2->connect(node4);
    node3->connect(node4);

    ppl.reorganize();

    while (true) {
        auto req = std::make_shared<vtf::pipeline::PipeRequest>(PipelineScenario::SAT, false);
        ppl.submit(req);
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(33));
    }
}

int main()
{
    // testDispatch();
    testPipeline();
}