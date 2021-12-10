/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 17:56:49
 * @LastEditors: yeonon
 * @LastEditTime: 2021-12-10 23:37:31
 */
#include "../core/pipeline/pipedata.hpp"
#include "../core/pipeline/pipenode_dispatcher.hpp"
#include "../core/pipeline/pipeline.hpp"
#include "../core/notifier.hpp"
#include "../core/utils/time_util.hpp"

using namespace vtf::pipeline;

enum MyScenario {
    Scenario1 = 100,
    Scenario2,
    Scenario3
};

enum NodeId {
    P1NODE = 0,
    P2NODE,
    P3NODE,
    MDPNODE,
    WPENODE,
};

void testDispatch()
{
    // vtf::pipeline::PipeNodeDispatcher<PipelineRequest> pd(8);

    // while (true) {
    //     std::shared_ptr<PipelineRequest> req = std::make_shared<PipelineRequest>(MyScenario::Scenario1);
    //     pd.queueInDispacther(req);
    //     std::this_thread::sleep_until(vtf::util::TimeUtil::awake_time(300));
    // }
}

class MypipeData : public vtf::pipeline::PipeData {

public:
    MypipeData(PipelineScenario scenario, int d)
        :vtf::pipeline::PipeData(scenario, true),
        data(d) 
    {}

    int outputD() { return data; }
private:
    int data;
};

void testPipeline()
{
    using PipelineRequest = MypipeData;

    PipeLine<PipelineRequest>::ConfigureTable table = 
    {
        .pipeNodeCreateInfos = 
        {
            {
                .id = NodeId::P1NODE,
                .name = "P1Node",
                .pipelineScenarios = {MyScenario::Scenario1, MyScenario::Scenario2},
                .processCallback = [](std::shared_ptr<PipelineRequest> request) -> bool {
                    VTF_LOGD("request {0} process P1 node", request->ID());
                    std::this_thread::sleep_until(vtf::utils::TimeUtil::awake_time(33));
                    return true;
                }
            },
            {
                .id = NodeId::P2NODE,
                .name = "P2Node",
                .pipelineScenarios = {MyScenario::Scenario1},
                .processCallback = [](std::shared_ptr<PipelineRequest> request) -> bool {
                    VTF_LOGD("request {0} process P2 node", request->ID());
                    std::this_thread::sleep_until(vtf::utils::TimeUtil::awake_time(33));
                    return true;
                }
            },
            {
                .id = NodeId::P3NODE,
                .name = "P3Node",
                .pipelineScenarios = {MyScenario::Scenario2},
                .processCallback = [](std::shared_ptr<PipelineRequest> request) -> bool {
                    VTF_LOGD("request {0} process P3 node", request->ID());
                    std::this_thread::sleep_until(vtf::utils::TimeUtil::awake_time(33));
                    return true;
                }
            },
            {
                NodeId::MDPNODE,
                "MDPNode",
                {MyScenario::Scenario1, MyScenario::Scenario2},
                [](std::shared_ptr<PipelineRequest> request) -> bool {
                    VTF_LOGD("request {0} process MDP node", request->ID());
                    std::this_thread::sleep_until(vtf::utils::TimeUtil::awake_time(33));
                    return true;
                }
            },
            {
                NodeId::WPENODE,
                "WPENode",
                {MyScenario::Scenario1, MyScenario::Scenario2},
                [](std::shared_ptr<PipelineRequest> request) -> bool {
                    VTF_LOGD("request {0} process WPE node", request->ID());
                    std::this_thread::sleep_until(vtf::utils::TimeUtil::awake_time(33));
                    return true;
                }
            }
        },
        .notifierCreateInfos = 
        {
            {
                .id = 1,
                .name = "pipeline_node_done_notifier",
                .processCallback = [](std::shared_ptr<PipelineRequest> request) {
                        if (request->getNotifyStatus() == vtf::NotifyStatus::ERROR) {
                            VTF_LOGE("node done {0} notify ERROR", request->ID());
                        } else {
                            VTF_LOGE("node done {0} notify OK", request->ID());
                        }
                        return true;
                },
                .type = vtf::NotifierType::DATA_LISTEN,
                .readyQueueSize = 8
            },
            {
                .id = 2,
                .name = "pipeline_result_notifier",
                .processCallback = [](std::shared_ptr<PipelineRequest> request) {
                        if (request->getNotifyStatus() == vtf::NotifyStatus::ERROR) {
                            VTF_LOGE("result {0} notify ERROR", request->ID());
                        } else {
                            VTF_LOGE("result {0} notify OK", request->ID());
                        }
                        return true;
                },
                .type = vtf::NotifierType::FINAL,
                .readyQueueSize = 8
            }
        },
        .nodeConnections = 
        {
            {P1NODE, P2NODE},
            {P1NODE, P3NODE},
            {P2NODE, MDPNODE},
            {P3NODE, MDPNODE}
        }
    };

    auto ppl = PipeLine<PipelineRequest>::generatePipeLineByConfigureTable(table);
    ppl->start();


    bool isSTop = false;
    int cnt = 0;
    while (true) {
        if (cnt == 100) {
            ppl->stop();

            VTF_LOGD("start stop");
            isSTop = true;
            VTF_LOGD("end stop");

            break;
        } else {
            std::this_thread::sleep_until(vtf::utils::TimeUtil::awake_time(33));
            if (!isSTop) {
                auto req = std::make_shared<PipelineRequest>(MyScenario::Scenario2, cnt);
                req->addNotifierForNode(P1NODE, 1);
                req->addNotifierForNode(P3NODE, 1);
                ppl->submit(req);
            }
        }
        cnt++;
    }
    VTF_LOGD("test pipeline stop");
}

int main()
{
    // testDispatch();
    testPipeline();
    // while (true) {
        
    // }
}