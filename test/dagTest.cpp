#include <cflow/core/dag.hpp>
#include <cflow/core/task/tftask.hpp>
#include <cflow/core/utils/thread/threadPool.hpp>
#include <cflow/core/type.hpp>
#include <cflow/core/utils/memory/buffer_manager.hpp>
#include <cflow/core/utils/memory/buffer_manager_factory.hpp>

#include <chrono>
#include <sstream>

#include <any>

using namespace cflow::utils::memory;

void testDAGbasic()
{
    std::shared_ptr<cflow::DAGNode> node1 = std::make_shared<cflow::DAGNode>(1);
    std::shared_ptr<cflow::DAGNode> node2 = std::make_shared<cflow::DAGNode>(2);
    std::shared_ptr<cflow::DAGNode> node3 = std::make_shared<cflow::DAGNode>(3);
    std::shared_ptr<cflow::DAGNode> node4 = std::make_shared<cflow::DAGNode>(4);
    std::shared_ptr<cflow::DAGNode> node5 = std::make_shared<cflow::DAGNode>(5);
    std::shared_ptr<cflow::DAGNode> node6 = std::make_shared<cflow::DAGNode>(6);
    std::shared_ptr<cflow::DAGNode> node7 = std::make_shared<cflow::DAGNode>(7);

    node1->connect(node2);
    node1->connect(node3);
    node2->connect(node4);
    // node3->connect(node4);
    node4->connect(node5);
    node4->connect(node6);

    // node1->connect(node2);
    // node1->connect(node3);
    // node2->connect(node4);
    // node3->connect(node4);

    cflow::DAG dag;
    dag.addNode(node1);
    dag.addNode(node2);
    dag.addNode(node3);
    dag.addNode(node4);
    dag.addNode(node5);
    dag.addNode(node6);
    // dag.addNode(node7);

    dag.buildGraph();

    dag.dumpGraph();

    auto              order = dag.multiTopologicalSort();
    std::stringstream ss;
    ss << "[";

    for (auto&& pipeline : order)
    {
        ss << "[";
        for (auto&& nodeId : pipeline)
        {
            ss << nodeId << ",";
        }
        ss << "]";
    }
    ss << "]";
    std::cout << ss.str() << std::endl;
}

void testTaskDag()
{
    std::shared_ptr<BufferManagerFactory<void>> bufferMgrFactory =
        std::make_shared<BufferManagerFactory<void>>();

    // cflow::task::TFTask task1("task_1");
    std::shared_ptr<cflow::task::TFTask> task1 =
        std::make_shared<cflow::task::TFTask>(bufferMgrFactory);
    task1->setProcessFunc([](int a, int b) { return a + b; }, 1, 2);

    std::shared_ptr<cflow::task::TFTask> task2 =
        std::make_shared<cflow::task::TFTask>(bufferMgrFactory);
    task2->setProcessFunc([](int a, int b) { return a + b; }, 1, 2);

    std::shared_ptr<cflow::task::TFTask> task3 =
        std::make_shared<cflow::task::TFTask>(bufferMgrFactory);
    task3->setProcessFunc([](int a, int b) { return a + b; }, 1, 2);

    std::shared_ptr<cflow::task::TFTask> task4 =
        std::make_shared<cflow::task::TFTask>(bufferMgrFactory);
    task4->setProcessFunc([](int a, int b) { return a + b; }, 1, 2);

    std::shared_ptr<cflow::task::TFTask> task5 =
        std::make_shared<cflow::task::TFTask>(bufferMgrFactory);
    task5->setProcessFunc([](int a, int b) { return a + b; }, 1, 2);
    task5->setPriority(cflow::TaskPriority::NORMAL);

    std::shared_ptr<cflow::task::TFTask> task6 =
        std::make_shared<cflow::task::TFTask>(bufferMgrFactory);

    task6->setProcessFunc([](int a, int b) { return a + b; }, 1, 2);

    std::shared_ptr<cflow::task::TFTask> task7 =
        std::make_shared<cflow::task::TFTask>(bufferMgrFactory);
    task7->setProcessFunc([](int a, int b) { return a + b; }, 1, 2);

    task4->connect(task3);
    task4->connect(task6);
    task3->connect(task6);
    task3->connect(task2);
    task6->connect(task5);
    task6->connect(task1);
    task1->connect(task2);
    task5->connect(task7);
    task1->connect(task7);
    task2->connect(task7);

    cflow::DAG dag;
    dag.addNode(task1);
    dag.addNode(task2);
    dag.addNode(task3);
    dag.addNode(task4);
    dag.addNode(task5);
    dag.addNode(task6);
    dag.addNode(task7);

    dag.buildGraph();
    dag.dumpGraph();
    dag.topologicalSort();

    dag.rebuildGraphIfNeed();
    dag.dumpGraph();
    dag.topologicalSort();
}

auto now() { return std::chrono::steady_clock::now(); }

auto awake_time()
{
    using std::chrono::operator""ms;
    return now() + 200ms;
}

template <typename T>
constexpr T convertTime(std::chrono::duration<double> originTime)
{
    return std::chrono::duration_cast<T>(originTime);
}

void testTaskExecute()
{
    std::shared_ptr<BufferManagerFactory<void>> bufferMgrFactory =
        std::make_shared<BufferManagerFactory<void>>();

    std::shared_ptr<cflow::task::TFTask> task1 =
        std::make_shared<cflow::task::TFTask>(bufferMgrFactory);
    task1->setProcessFunc([](int a, int b) { return a + b; }, 1, 2);

    std::shared_ptr<cflow::task::TFTask> task2 =
        std::make_shared<cflow::task::TFTask>(bufferMgrFactory);
    task2->setProcessFunc([](int a, int b) { return a + b; }, 1, 2);

    std::shared_ptr<cflow::task::TFTask> task3 =
        std::make_shared<cflow::task::TFTask>(bufferMgrFactory);
    task3->setProcessFunc([](int a, int b) { return a + b; }, 1, 2);

    std::shared_ptr<cflow::task::TFTask> task4 =
        std::make_shared<cflow::task::TFTask>(bufferMgrFactory);
    task4->setProcessFunc([](int a, int b) { return a + b; }, 1, 2);

    std::shared_ptr<cflow::task::TFTask> task5 =
        std::make_shared<cflow::task::TFTask>(bufferMgrFactory);
    task5->setProcessFunc([](int a, int b) { return a + b; }, 1, 2);
    task5->setPriority(cflow::TaskPriority::URGENCY);

    std::shared_ptr<cflow::task::TFTask> task6 =
        std::make_shared<cflow::task::TFTask>(bufferMgrFactory);

    task6->setProcessFunc([](int a, int b) { return a + b; }, 1, 2);

    std::shared_ptr<cflow::task::TFTask> task7 =
        std::make_shared<cflow::task::TFTask>(bufferMgrFactory);
    task7->setProcessFunc([](int a, int b) { return a + b; }, 1, 2);

    task4->connect(task3);
    task4->connect(task6);
    task3->connect(task6);
    task3->connect(task2);
    task6->connect(task5);
    task6->connect(task1);
    task1->connect(task2);
    task5->connect(task7);
    task1->connect(task7);
    task2->connect(task7);

    cflow::DAG dag;
    dag.addNode(task1);
    dag.addNode(task2);
    dag.addNode(task3);
    dag.addNode(task4);
    dag.addNode(task5);
    dag.addNode(task6);
    dag.addNode(task7);

    std::unordered_map<long, std::shared_ptr<cflow::task::TFTask>> taskMap;
    taskMap[task1->ID()] = task1;
    taskMap[task2->ID()] = task2;
    taskMap[task3->ID()] = task3;
    taskMap[task4->ID()] = task4;
    taskMap[task5->ID()] = task5;
    taskMap[task6->ID()] = task6;
    taskMap[task7->ID()] = task7;

    dag.buildGraph();

    cflow::utils::thread::ThreadPool pool(4);

    auto                           start     = std::chrono::system_clock::now();
    std::vector<std::vector<long>> topoOrder = dag.topologicalSort();
    for (auto& order : topoOrder)
    {
        std::vector<std::future<void>> futureList;
        for (long taskId : order)
        {
            futureList.push_back(
                pool.emplace(taskMap[taskId]->getProcessFunc()));
        }

        for (std::future<void>& future : futureList)
        {
            future.get();
        }
    }
    auto                          end     = std::chrono::system_clock::now();
    std::chrono::duration<double> runTime = end - start;
    CFLOW_LOGE("all task need: {0}ms",
               convertTime<std::chrono::milliseconds>(runTime).count());
}

int main()
{
    testDAGbasic();
    testTaskDag();
    testTaskExecute();
}