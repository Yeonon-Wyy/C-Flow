/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 17:56:49
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-24 23:18:24
 */
#include "../core/pipeline/pipeRequest.hpp"
#include "../core/pipeline/pipeNodeDispatcher.hpp"
#include "../core/pipeline/pipeline.hpp"
#include "../core/notifier.hpp"

using namespace vtf::pipeline;

enum MyScenario {
    Scenario1,
    Scenario2,
    Scenario3
};

void testDispatch()
{
    // vtf::pipeline::PipeNodeDispatcher<vtf::pipeline::PipeRequest> pd(8);

    // while (true) {
    //     std::shared_ptr<vtf::pipeline::PipeRequest> req = std::make_shared<vtf::pipeline::PipeRequest>(MyScenario::Scenario1);
    //     pd.queueInDispacther(req);
    //     std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(300));
    // }
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
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(33));
        return true;
    });

    auto node3 = ppl.addPipeNode("P3Node", [](std::shared_ptr<vtf::pipeline::Request> request) -> bool {
        VTF_LOGD("request {0} process P3 node", request->ID());
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(200));
        VTF_LOGD("request {0} process P3 node done", request->ID());

        return true;
    });

    auto node4 = ppl.addPipeNode("MDP", [](std::shared_ptr<vtf::pipeline::Request> request) -> bool {
        VTF_LOGD("request {0} process MDP node", request->ID());
        std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(1000));
        VTF_LOGD("request {0} process MDP node done", request->ID());
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

    
    ppl.addNotifier(
        {
            "pipeline_result_notifier",
            [](std::shared_ptr<vtf::pipeline::Request> request) {
                    if (request->getNotifyStatus() == vtf::NotifyStatus::ERROR) {
                        VTF_LOGE("result {0} notify ERROR", request->ID());
                    } else {
                        VTF_LOGE("result {0} notify OK", request->ID());
                    }
                    return true;
            },
            vtf::NotifierType::FINAL,
            8
        }
    );

    ppl.addNotifier(
        {
            "pipeline_node_done_notifier",
            [](std::shared_ptr<vtf::pipeline::Request> request) {
                    if (request->getNotifyStatus() == vtf::NotifyStatus::ERROR) {
                        VTF_LOGE("node done {0} notify ERROR", request->ID());
                    } else {
                        VTF_LOGE("node done {0} notify OK", request->ID());
                    }
                    return true;
            },
            vtf::NotifierType::DATA_LISTEN,
            8
        }
    );
    ppl.start();

    bool isSTop = false;
    int cnt = 0;
    while (true) {
        if (cnt == 100) {
            ppl.stop();

            VTF_LOGD("start stop");
            isSTop = true;
            VTF_LOGD("end stop");

            break;
        } else {
            std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(33));
            if (!isSTop) {
                auto req = std::make_shared<vtf::pipeline::PipeRequest>(MyScenario::Scenario2, false);
                ppl.submit(req);
            }
        }
        cnt++;
    }
    VTF_LOGD("test pipeline stop");
    VTF_LOGD("node1 useCount {0}", node1.use_count());
}

int main()
{
    // testDispatch();
    testPipeline();
    // while (true) {
        
    // }
}