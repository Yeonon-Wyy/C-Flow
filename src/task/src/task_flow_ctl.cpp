#include "task_flow_ctl.h"
#include "../../utils/include/log.h"
#include "../../utils/include/dumper.h"
#include "tftask.h"

namespace cflow::task {

TaskThreadPool::TaskThreadPool(size_t threadSize) : isStop(false)
{
    for (size_t i = 0; i < threadSize; i++)
    {
        m_workers.emplace_back([this]() {
            while (true)
            {
                std::shared_ptr<TFTask> task;

                {
                    std::unique_lock<std::mutex> lk(this->m_taskMutex);
                    this->m_taskCV.wait(lk, [this]() {
                        return this->isStop || !m_tasks.empty();
                    });
                    // if isStop flag is ture and task queue is empty, we just
                    // return else we need execute task, even if isStop flag is
                    // true
                    if (this->isStop && m_tasks.empty())
                    {
                        return;
                    }
                    task = std::move(this->m_tasks.top());
                    this->m_tasks.pop();
                }
                // execute task
                (*task)();
            }
        });
    }
}

auto TaskThreadPool::emplaceTask(std::shared_ptr<TFTask> task)
    -> std::future<void>
{
    auto pt =
        std::make_shared<std::packaged_task<void()>>(task->getProcessFunc());

    // set runable, threadPool will execute this function
    task->setProcessFunc([pt]() { (*pt)(); });

    std::future<void> taskFuture = pt->get_future();
    {
        std::unique_lock<std::mutex> lk(this->m_taskMutex);
        if (isStop)
        {
            throw std::runtime_error("emplace on stopped ThreadPool");
        }
        this->m_tasks.emplace(task);
    }

    this->m_taskCV.notify_one();
    return taskFuture;
}

TaskThreadPool::~TaskThreadPool()
{
    {
        std::unique_lock<std::mutex> lk(this->m_taskMutex);
        isStop = true;
    }

    // notify all wait thread, let remain thread can run
    this->m_taskCV.notify_all();

    // wait all thread run complate
    for (auto& worker : this->m_workers)
    {
        worker.join();
    }
}

TaskFlowCtl::TaskFlowCtl(bool enableDebug)
    : m_threadPool(TASKFLOWCTL_THREADPOOL_MAX_THREAD),
      m_dag(),
      m_bufferMgrFactory(std::make_shared<BufferManagerFactory<void>>()),
      m_debugEnable(enableDebug)
{
}

void TaskFlowCtl::reorganizeTaskOrder()
{
    m_taskOrder = m_dag.topologicalSort();
    if (!m_debugEnable) return;
    {
        uint32_t curSceneraio = 0;
        std::unordered_map<uint32_t, std::vector<std::string>>
                                           scenario2TaskOrderWithNameMap;
        std::set<std::vector<std::string>> pipelineWithNameSet;
        for (auto&& order : m_taskOrder)
        {
            std::vector<std::string> taskOrderWithName;
            for (auto&& taskID : order)
            {
                std::cout << taskID << " ";
                taskOrderWithName.push_back(m_taskIdMap[taskID]->name());
            }
            std::cout << std::endl;
            scenario2TaskOrderWithNameMap[curSceneraio] = taskOrderWithName;
            curSceneraio++;
        }
        cflow::utils::Dumper dumper("Task_flow_control",
                                    scenario2TaskOrderWithNameMap,
                                    DUMPTYPE::TASKFLOW);
        std::string          filename = "Task flow control.dot";
        std::fstream         fs(filename, std::ios::out | std::ios::trunc);
        dumper.dumpDOT(fs);
    }
}

void TaskFlowCtl::commonSetting(std::shared_ptr<TFTask> task)
{
    m_taskIdMap.emplace(task->ID(), task);
    m_dag.addNode(task);
}

void TaskFlowCtl::start()
{
    reorganizeTaskOrder();
    for (auto& curLevelTaskIds : m_taskOrder)
    {
        std::vector<std::pair<std::string, std::future<void>>> taskfutureList;
        for (cflow_id_t taskId : curLevelTaskIds)
        {
            if (m_taskIdMap.count(taskId) == 0)
            {
                std::cerr << "can't find the task id(" << taskId
                          << ") in taskIdMap, please check it." << std::endl;
                return;
            }
            std::shared_ptr<TFTask> task = m_taskIdMap[taskId];
            taskfutureList.emplace_back(task->name(),
                                        m_threadPool.emplaceTask(task));
        }

        for (std::pair<std::string, std::future<void>>& taskfuturePair :
             taskfutureList)
        {
            taskfuturePair.second.get();
            CFLOW_LOGE("{0} execute complate!", taskfuturePair.first);
        }
    }
}

} // namespace cflow::task