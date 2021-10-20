#include "../core/dag.hpp"
#include "../core/task.hpp"
#include "../core/threadPool.hpp"

#include <chrono>

#include <any>

void testDAGbasic()
{
    std::cout << "testDAGbasic:\n";

    
    std::shared_ptr<vtf::DAGNode> node1 = std::make_shared<vtf::DAGNode>(1);
    std::shared_ptr<vtf::DAGNode> node2 = std::make_shared<vtf::DAGNode>(2);
    std::shared_ptr<vtf::DAGNode> node3 = std::make_shared<vtf::DAGNode>(3);
    std::shared_ptr<vtf::DAGNode> node4 = std::make_shared<vtf::DAGNode>(4);
    std::shared_ptr<vtf::DAGNode> node5 = std::make_shared<vtf::DAGNode>(5);
    std::shared_ptr<vtf::DAGNode> node6 = std::make_shared<vtf::DAGNode>(6);

    node4->precede(node3);
    node4->precede(node6);
    node3->precede(node6);
    node3->precede(node2);
    node6->precede(node5);
    node6->precede(node1);
    node1->precede(node2);


    vtf::DAG dag;
    dag.addNode(node1);
    dag.addNode(node2);
    dag.addNode(node3);
    dag.addNode(node4);
    dag.addNode(node5);
    dag.addNode(node6);



    dag.buildGraph();

    dag.dumpGraph();

    dag.topologicalSort();
}

void testTaskDag()
{
    std::cout << "testTaskDag:\n";

    // vtf::Task task1("task_1");
    std::shared_ptr<vtf::Task> task1 = std::make_shared<vtf::Task>("task_1");
    auto pt1 = task1->commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::Task> task2 = std::make_shared<vtf::Task>("task_2");
    auto pt2 = task2->commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::Task> task3 = std::make_shared<vtf::Task>("task_3");
    auto pt3 = task3->commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::Task> task4 = std::make_shared<vtf::Task>("task_4");
    auto pt4 = task4->commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::Task> task5 = std::make_shared<vtf::Task>("task_5");
    auto pt5 = task5->commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);
    task5->setPriority(vtf::TaskPriority::URGENCY);

    std::shared_ptr<vtf::Task> task6 = std::make_shared<vtf::Task>("task_6");

    auto pt6 = task6->commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::Task> task7 = std::make_shared<vtf::Task>("task_7");
    auto pt7 = task7->commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    task4->precede(task3);
    task4->precede(task6);
    task3->precede(task6);
    task3->precede(task2);
    task6->precede(task5);
    task6->precede(task1);
    task1->precede(task2);
    task5->precede(task7);  
    task1->precede(task7);
    task2->precede(task7);


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

    std::cout << "twitch" << std::endl;
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
    std::cout << "testTaskExecute:\n";

    std::shared_ptr<vtf::Task> task1 = std::make_shared<vtf::Task>("task_1");
    auto pt1 = task1->commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::Task> task2 = std::make_shared<vtf::Task>("task_2");
    auto pt2 = task2->commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::Task> task3 = std::make_shared<vtf::Task>("task_3");
    auto pt3 = task3->commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::Task> task4 = std::make_shared<vtf::Task>("task_4");
    auto pt4 = task4->commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::Task> task5 = std::make_shared<vtf::Task>("task_5");
    auto pt5 = task5->commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);
    task5->setPriority(vtf::TaskPriority::URGENCY);

    std::shared_ptr<vtf::Task> task6 = std::make_shared<vtf::Task>("task_6");

    auto pt6 = task6->commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::Task> task7 = std::make_shared<vtf::Task>("task_7");
    auto pt7 = task7->commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    task4->precede(task3);
    task4->precede(task6);
    task3->precede(task6);
    task3->precede(task2);
    task6->precede(task5);
    task6->precede(task1);
    task1->precede(task2);
    task5->precede(task7);  
    task1->precede(task7);
    task2->precede(task7);


    vtf::DAG dag;
    dag.addNode(task1);
    dag.addNode(task2);
    dag.addNode(task3);
    dag.addNode(task4);
    dag.addNode(task5);
    dag.addNode(task6);
    dag.addNode(task7);

    std::unordered_map<long, std::shared_ptr<vtf::Task>> taskMap;
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
    std::cout << "all task need: " << convertTime<std::chrono::milliseconds>(runTime).count() << "ms" << std::endl;


}

int main()
{
    testDAGbasic();
    testTaskDag();
    testTaskExecute();



}