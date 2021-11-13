/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 17:56:49
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-13 16:24:56
 */
#include "../core/pipeline/pipeRequest.hpp"
#include "../core/pipeline/pipeNodeDispatcher.hpp"
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
    auto node1 = ppl.addPipeNode("P1Node", [](std::shared_ptr<vtf::pipeline::Request> request) -> bool {
        VTF_LOGD("request {0} process P1 node", request->ID());
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(20));
        return true;
    });

    auto node2 = ppl.addPipeNode("P2Node", [](std::shared_ptr<vtf::pipeline::Request> request) -> bool {
        VTF_LOGD("request {0} process P2 node", request->ID());
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(20));
        return true;
    });

    auto node3 = ppl.addPipeNode("P3Node", [](std::shared_ptr<vtf::pipeline::Request> request) -> bool {
        VTF_LOGD("request {0} process P3 node", request->ID());
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(20));
        return true;
    });

    auto node4 = ppl.addPipeNode("MDP", [](std::shared_ptr<vtf::pipeline::Request> request) -> bool {
        VTF_LOGD("request {0} process MDP node", request->ID());
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(20));
        return true;
    });

    auto node5 = ppl.addPipeNode("WPE", [](std::shared_ptr<vtf::pipeline::Request> request) -> bool {
        VTF_LOGD("request {0} process WPE node", request->ID());
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(20));
        return true;
    });

    //[[1],[2,3],[4]]
    node1->addScenarios({PipelineScenario::SAT,PipelineScenario::BOKEH});
    node2->addScenarios(PipelineScenario::SAT);
    node3->addScenarios(PipelineScenario::BOKEH);
    node4->addScenarios({PipelineScenario::SAT,PipelineScenario::BOKEH});

    node1->connect(node2);
    node1->connect(node3);
    node2->connect(node4);
    node3->connect(node4);

    ppl.constructPipelinesByScenario();

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