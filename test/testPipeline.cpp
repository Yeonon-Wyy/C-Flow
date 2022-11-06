/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-10-30 17:56:49
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-11-06 22:10:25
 */
#include "../src/core/notifier.hpp"
#include "../src/core/pipeline/pipe_task.hpp"
#include "../src/core/pipeline/pipeline.hpp"
#include "../src/core/pipeline/pipenode_dispatcher.hpp"
#include "../src/core/utils/log/trace_log.hpp"
#include "../src/core/utils/time_util.hpp"

using namespace cflow::pipeline;

enum MyScenario
{
    Scenario1 = 100,
    Scenario2,
    Scenario3,
    Scenario4
};

enum NodeId
{
    P1NODE = 0,
    P2NODE,
    P3NODE,
    MDPNODE,
    WPENODE,
};

void testDispatch()
{
    // cflow::pipeline::PipeNodeDispatcher<PipelineRequest> pd(8);

    // while (true) {
    //     std::shared_ptr<PipelineRequest> req =
    //     std::make_shared<PipelineRequest>(MyScenario::Scenario1);
    //     pd.queueInDispacther(req);
    //     std::this_thread::sleep_until(cflow::util::TimeUtil::awake_time(300));
    // }
}

using namespace cflow::utils::memory;

class MypipeTask : public cflow::pipeline::PipeTask
{
public:
    MypipeTask(PipelineScenario scenario, int d)
        : cflow::pipeline::PipeTask(scenario, true),
          task(d)
    {
    }

    bool constructIO() override
    {
        if (scenario() == MyScenario::Scenario2)
        {
            addOutputForNode(0, BufferSpecification{.sizeOfBytes  = 16,
                                                    .minQueueSize = 8,
                                                    .maxQueueSize = 10,
                                                    .name = "node0-ouput"});
            addInputForNode(2, BufferSpecification{.sizeOfBytes  = 16,
                                                   .minQueueSize = 8,
                                                   .maxQueueSize = 10,
                                                   .name = "node0-ouput"});
            addOutputForNode(2, BufferSpecification{.sizeOfBytes  = 16,
                                                    .minQueueSize = 8,
                                                    .maxQueueSize = 10,
                                                    .name = "node2-ouput"});
            addInputForNode(3, BufferSpecification{.sizeOfBytes  = 16,
                                                   .minQueueSize = 8,
                                                   .maxQueueSize = 10,
                                                   .name = "node2-ouput"});
            addOutputForNode(3, BufferSpecification{.sizeOfBytes  = 16,
                                                    .minQueueSize = 8,
                                                    .maxQueueSize = 10,
                                                    .name = "node3-ouput"});
        }
        return true;
    }

    int outputD() { return task; }

private:
    int task;
};

void testPipeline()
{
    using PipelineRequest = MypipeTask;

    // clang-format off
    PipeLine<PipelineRequest>::ConfigureTable table =
    {
        .threadPoolSize = 8,
        .pipeNodeCreateInfos =
        {
            {  
                .id                = NodeId::P1NODE,
                .name              = "FirstNode",
                .pipelineScenarios = {MyScenario::Scenario1,
                                     MyScenario::Scenario2,
                                     MyScenario::Scenario3,
                                     MyScenario::Scenario4},
                .processCallback = [](std::shared_ptr<PipelineRequest> request) -> bool 
                {
                   TRACE_FUNC_ID_START("P1NodeProcess", request->ID());
                   CFLOW_LOGD("request {0} process P1 node", request->ID());
                   if (request->ID() % 2 == 0)
                   {
                       CFLOW_LOGD("return error");
                       return false;
                   }
                   if (request->getTaskType() == cflow::TaskType::RT)
                       std::this_thread::sleep_until(
                           cflow::utils::TimeUtil::awake_time(1));
                   else
                       std::this_thread::sleep_until(
                           cflow::utils::TimeUtil::awake_time(1));
                   for (auto &&out : request->output())
                   {
                       *((int *)out->ptr) = 0;
                   }
                   return true;
                }
            },
            {
                .id                = NodeId::P2NODE,
                .name              = "FDNode",
                .pipelineScenarios = {MyScenario::Scenario1,
                                     MyScenario::Scenario3},
                .processCallback   = [](std::shared_ptr<PipelineRequest> request) -> bool 
                {
                   CFLOW_LOGD("request {0} process P2 node", request->ID());
                   std::this_thread::sleep_until(
                       cflow::utils::TimeUtil::awake_time(1));
                   return true;
                }
            },
            {
                .id                = NodeId::P3NODE,
                .name              = "WaterMarkNode",
                .pipelineScenarios = {MyScenario::Scenario2},
                .processCallback = [](std::shared_ptr<PipelineRequest> request) -> bool 
                {
                   CFLOW_LOGD("request {0} process P3 node", request->ID());
                   std::this_thread::sleep_until(
                       cflow::utils::TimeUtil::awake_time(1));
                   int sz = request->input().size();
                   for (int i = 0; i < sz; i++)
                   {
                       auto in            = request->input()[i];
                       auto out           = request->output()[i];
                       *((int *)out->ptr) = (*((int *)in->ptr)) + 1;
                   }
                   return true;
                }
            },
            {
                NodeId::MDPNODE,
                "ScaleNode",
                {MyScenario::Scenario1, MyScenario::Scenario2},
                [](std::shared_ptr<PipelineRequest> request) -> bool 
                {
                   CFLOW_LOGD("request {0} process MDP node", request->ID());
                   if (request->getTaskType() == cflow::TaskType::RT)
                       std::this_thread::sleep_until(
                           cflow::utils::TimeUtil::awake_time(1));
                   else
                       std::this_thread::sleep_until(
                           cflow::utils::TimeUtil::awake_time(1));
                   int sz = request->input().size();
                   for (int i = 0; i < sz; i++)
                   {
                       auto in            = request->input()[i];
                       auto out           = request->output()[i];
                       *((int *)out->ptr) = (*((int *)in->ptr)) + 1;
                   }

                   for (auto &&out : request->output())
                   {
                       CFLOW_LOGD("last output val : {0}", *((int *)out->ptr));
                   }
                   return true;
                }
            },
            {
                NodeId::WPENODE,
                "AlgoNode",
                {MyScenario::Scenario1, MyScenario::Scenario2},
                [](std::shared_ptr<PipelineRequest> request) -> bool 
                {
                   CFLOW_LOGD("request {0} process WPE node", request->ID());
                   std::this_thread::sleep_until(
                       cflow::utils::TimeUtil::awake_time(1));
                   return true;
                }
            }
        },
        .notifierCreateInfos =
        {
            {
                .id   = 1,
                .name = "pipeline_node_done_notifier",
                .processCallback = [](std::shared_ptr<PipelineRequest> request) 
                {
                    if (request->getNotifyStatus() ==
                        cflow::NotifyStatus::ERROR)
                    {
                        CFLOW_LOGE("node done {0} notify ERROR",
                                    request->ID());
                    }
                    else
                    {
                        CFLOW_LOGE("node done {0} notify OK",
                                    request->ID());
                    }
                    return true;
                },
                .configProgress = []() 
                {
                    CFLOW_LOGD("pipeline_node_done_notifier - user "
                                "define config");
                },
                .stopProgress = []() 
                {
                    CFLOW_LOGD(
                        "pipeline_node_done_notifier - user define stop");
                },
                .type = cflow::NotifierType::TASK_LISTEN,
            },
            {
                .id   = 2,
                .name = "pipeline_result_notifier",
                .processCallback = [](std::shared_ptr<PipelineRequest> request) 
                {
                    if (request->getNotifyStatus() ==
                        cflow::NotifyStatus::ERROR)
                    {
                        CFLOW_LOGE("result {0} notify ERROR",
                                    request->ID());
                    }
                    else
                    {
                        CFLOW_LOGE("result {0} notify OK", request->ID());
                    }
                    return true;
                },
                .type = cflow::NotifierType::FINAL,
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
    // clang-format on

    auto ppl =
        PipeLine<PipelineRequest>::generatePipeLineByConfigureTable(table);
    ppl->start();

    bool isSTop = false;
    int  cnt    = 0;
    while (true)
    {
        if (cnt == 100)
        {
            ppl->stop();

            CFLOW_LOGD("start stop");
            CFLOW_LOGD("end stop");

            break;
        }
        else
        {
            std::this_thread::sleep_until(
                cflow::utils::TimeUtil::awake_time(33));
            if (!isSTop)
            {
                auto req = std::make_shared<PipelineRequest>(
                    MyScenario::Scenario2, cnt);
                // if (req->ID() % 2 == 0) {
                req->setTaskType(cflow::TaskType::RT);
                // }
                req->addNotifierForNode(1, P1NODE);
                req->addNotifierForNode(1, P3NODE);
                ppl->submit(req);
            }
        }
        cnt++;
    }

    CFLOW_LOGD("test pipeline stop");
}

int main()
{
    // testDispatch();
    testPipeline();
    // while (true) {

    // }
}