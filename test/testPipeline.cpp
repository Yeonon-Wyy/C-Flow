/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 17:56:49
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-07-02 17:01:18
 */
#include "../src/core/pipeline/pipedata.hpp"
#include "../src/core/pipeline/pipenode_dispatcher.hpp"
#include "../src/core/pipeline/pipeline.hpp"
#include "../src/core/notifier.hpp"
#include "../src/core/utils/time_util.hpp"

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

using namespace vtf::utils::memory;

class MypipeData : public vtf::pipeline::PipeData {

public:
    MypipeData(PipelineScenario scenario, int d)
        :vtf::pipeline::PipeData(scenario, true),
        data(d) 
    {}

    bool constructIO() override 
    {
        if (scenario() == MyScenario::Scenario2) {
            addOutputForNode(0, BufferSpecification{
                .sizeOfBytes = 16,
                .minQueueSize = 8,
                .maxQueueSize = 10,
                .name = "node0-ouput"
            });
            addInputForNode(2, BufferSpecification{
                .sizeOfBytes = 16,
                .minQueueSize = 8,
                .maxQueueSize = 10,
                .name = "node0-ouput"
            });
            addOutputForNode(2, BufferSpecification{
                .sizeOfBytes = 16,
                .minQueueSize = 8,
                .maxQueueSize = 10,
                .name = "node2-ouput"
            });
            addInputForNode(3, BufferSpecification{
                .sizeOfBytes = 16,
                .minQueueSize = 8,
                .maxQueueSize = 10,
                .name = "node2-ouput"
            });
            addOutputForNode(3, BufferSpecification{
                .sizeOfBytes = 16,
                .minQueueSize = 8,
                .maxQueueSize = 10,
                .name = "node3-ouput"
            });
        }
        return true;
    }    

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
                    if (request->getDataType() == vtf::DataType::DATATYPE_RT)
                        std::this_thread::sleep_until(vtf::utils::TimeUtil::awake_time(33));
                    else
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
                    if (request->getDataType() == vtf::DataType::DATATYPE_RT)
                        std::this_thread::sleep_until(vtf::utils::TimeUtil::awake_time(33));
                    else
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
                .configProgress = []() {
                    VTF_LOGD("pipeline_node_done_notifier - user define config");  
                },
                .stopProgress = []() {
                    VTF_LOGD("pipeline_node_done_notifier - user define stop");
                },
                .type = vtf::NotifierType::DATA_LISTEN,
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
                if (req->ID() % 2 == 0) {
                    req->setDataType(vtf::DataType::DATATYPE_RT);
                }
                req->addNotifierForNode(P1NODE, 1);
                req->addNotifierForNode(P3NODE, 1);
                ppl->submit(req);
            }
        }
        cnt++;
    }

    // bool isSTop = false;
    // int cnt = 0;
    // while (true) {
    //     if (cnt == 100) {
    //         // ppl->stop();

    //         VTF_LOGD("start stop");
    //         isSTop = true;
    //         VTF_LOGD("end stop");

    //         break;
    //     } else {
    //         std::this_thread::sleep_until(vtf::utils::TimeUtil::awake_time(33));
    //         if (!isSTop) {
    //             auto req = std::make_shared<PipelineRequest>(MyScenario::Scenario2, cnt);
    //             // if (req->ID() % 2 == 0) {
    //                 req->setDataType(vtf::DataType::DATATYPE_RT);
    //             // }
    //             req->addNotifierForNode(P1NODE, 1);
    //             req->addNotifierForNode(P3NODE, 1);
    //             ppl->submit(req);
    //         }
    //     }
    //     cnt++;
    // }

    VTF_LOGD("test pipeline stop");
}

int main()
{
    // testDispatch();
    testPipeline();
    // while (true) {
        
    // }
}