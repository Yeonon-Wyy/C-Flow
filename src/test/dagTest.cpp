#include "../core/dag.hpp"
#include "../core/task/task.hpp"
#include "../core/threadPool.hpp"

#include <chrono>

#include <any>

void testDAGbasic()
{

    
    std::shared_ptr<vtf::DAGNode> node1 = std::make_shared<vtf::DAGNode>(1);
    std::shared_ptr<vtf::DAGNode> node2 = std::make_shared<vtf::DAGNode>(2);
    std::shared_ptr<vtf::DAGNode> node3 = std::make_shared<vtf::DAGNode>(3);
    std::shared_ptr<vtf::DAGNode> node4 = std::make_shared<vtf::DAGNode>(4);
    std::shared_ptr<vtf::DAGNode> node5 = std::make_shared<vtf::DAGNode>(5);
    std::shared_ptr<vtf::DAGNode> node6 = std::make_shared<vtf::DAGNode>(6);
    std::shared_ptr<vtf::DAGNode> node7 = std::make_shared<vtf::DAGNode>(7);


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

    vtf::DAG dag;
    dag.addNode(node1);
    dag.addNode(node2);
    dag.addNode(node3);
    dag.addNode(node4);
    dag.addNode(node5);
    dag.addNode(node6);
    // dag.addNode(node7);


    dag.buildGraph();

    dag.dumpGraph();

    auto order = dag.multiTopologicalSort();
}

void testTaskDag()
{

    // vtf::task::Task task1("task_1");
    std::shared_ptr<vtf::task::Task> task1 = std::make_shared<vtf::task::Task>();
    auto pt1 = task1->commit([](int a, int b) {
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::task::Task> task2 = std::make_shared<vtf::task::Task>();
    auto pt2 = task2->commit([](int a, int b) {
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::task::Task> task3 = std::make_shared<vtf::task::Task>();
    auto pt3 = task3->commit([](int a, int b) {
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::task::Task> task4 = std::make_shared<vtf::task::Task>();
    auto pt4 = task4->commit([](int a, int b) {
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::task::Task> task5 = std::make_shared<vtf::task::Task>();
    auto pt5 = task5->commit([](int a, int b) {
        return a + b;
    }, 1, 2);
    task5->setPriority(vtf::task::TaskPriority::URGENCY);

    std::shared_ptr<vtf::task::Task> task6 = std::make_shared<vtf::task::Task>();

    auto pt6 = task6->commit([](int a, int b) {
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::task::Task> task7 = std::make_shared<vtf::task::Task>();
    auto pt7 = task7->commit([](int a, int b) {
        return a + b;
    }, 1, 2);

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


    vtf::DAG dag;
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
 
auto awake_time() {
    using std::chrono::operator""ms;
    return now() + 200ms;
}

template<typename T>
constexpr T convertTime(std::chrono::duration<double> originTime)
{
    return std::chrono::duration_cast<T>(originTime);
}

void testTaskExecute()
{

    std::shared_ptr<vtf::task::Task> task1 = std::make_shared<vtf::task::Task>();
    auto pt1 = task1->commit([](int a, int b) {
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::task::Task> task2 = std::make_shared<vtf::task::Task>();
    auto pt2 = task2->commit([](int a, int b) {
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::task::Task> task3 = std::make_shared<vtf::task::Task>();
    auto pt3 = task3->commit([](int a, int b) {
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::task::Task> task4 = std::make_shared<vtf::task::Task>();
    auto pt4 = task4->commit([](int a, int b) {
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::task::Task> task5 = std::make_shared<vtf::task::Task>();
    auto pt5 = task5->commit([](int a, int b) {
        return a + b;
    }, 1, 2);
    task5->setPriority(vtf::task::TaskPriority::URGENCY);

    std::shared_ptr<vtf::task::Task> task6 = std::make_shared<vtf::task::Task>();

    auto pt6 = task6->commit([](int a, int b) {
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::task::Task> task7 = std::make_shared<vtf::task::Task>();
    auto pt7 = task7->commit([](int a, int b) {
        return a + b;
    }, 1, 2);

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


    vtf::DAG dag;
    dag.addNode(task1);
    dag.addNode(task2);
    dag.addNode(task3);
    dag.addNode(task4);
    dag.addNode(task5);
    dag.addNode(task6);
    dag.addNode(task7);

    std::unordered_map<long, std::shared_ptr<vtf::task::Task>> taskMap;
    taskMap[task1->getID()] = task1;
    taskMap[task2->getID()] = task2;
    taskMap[task3->getID()] = task3;
    taskMap[task4->getID()] = task4;
    taskMap[task5->getID()] = task5;
    taskMap[task6->getID()] = task6;
    taskMap[task7->getID()] = task7;


    dag.buildGraph();

    vtf::ThreadPool pool(4);


    auto start = std::chrono::system_clock::now();
    std::vector<std::vector<long>> topoOrder = dag.topologicalSort();
    for (auto& order : topoOrder) {
        std::vector<std::future<void>> futureList;
        for (long taskId : order) {
            futureList.push_back(pool.emplace(taskMap[taskId]->getTaskFunc()));
        }

        for (std::future<void> &future : futureList) {
            future.get();
        }
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> runTime = end - start;
    VTF_LOGI("all task need: {0}ms", convertTime<std::chrono::milliseconds>(runTime).count());


}

int main()
{
    testDAGbasic();
    // testTaskDag();
    // testTaskExecute();



}