/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 17:56:49
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-15 23:00:07
 */
#include "../core/pipeline/pipeRequest.hpp"
#include "../core/pipeline/pipeNodeDispatcher.hpp"
#include "../core/pipeline/pipeline.hpp"

using namespace vtf::pipeline;

enum MyScenario {
    Scenario1,
    Scenario2,
    Scenario3
};

void testDispatch()
{
    vtf::pipeline::PipeNodeDispatcher<vtf::pipeline::PipeRequest> pd(8);
    pd.run();

    while (true) {
        std::shared_ptr<vtf::pipeline::PipeRequest> req = std::make_shared<vtf::pipeline::PipeRequest>(MyScenario::Scenario1);
        pd.queueInDispacther(req);
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(300));
    }
}



void testPipeline()
{
    vtf::pipeline::PipeLine<vtf::pipeline::Request> ppl;
    auto node1 = ppl.addPipeNode("P1Node", [](std::shared_ptr<vtf::pipeline::Request> request) -> bool {
        VTF_LOGD("request {0} process P1 node", request->ID());
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
        return true;
    });

    auto node2 = ppl.addPipeNode("P2Node", [](std::shared_ptr<vtf::pipeline::Request> request) -> bool {
        VTF_LOGD("request {0} process P2 node", request->ID());
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
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

    node1->addScenarios({MyScenario::Scenario1, MyScenario::Scenario2});
    node2->addScenarios(MyScenario::Scenario1);
    node3->addScenarios(MyScenario::Scenario2);
    node4->addScenarios({MyScenario::Scenario1,MyScenario::Scenario2});

    node1->connect(node2);
    node1->connect(node3);
    node2->connect(node4);
    node3->connect(node4);

    ppl.start();

    // auto req = std::make_shared<vtf::pipeline::PipeRequest>(MyScenario::Scenario1, false);
    // ppl.submit(req);

    while (true) {
        auto req = std::make_shared<vtf::pipeline::PipeRequest>(MyScenario::Scenario2, false);
        // if (req->ID() == 100) {
        //     ppl.stop();
        //     continue;
        // } else {

            ppl.submit(req);
            std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(33));
        // }

    }
}

int main()
{
    // testDispatch();
    testPipeline();
}