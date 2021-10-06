#include "../core/Dag.hpp"
#include "../core/task.hpp"

void testDAGbasic()
{
    std::cout << "testDAGbasic:\n";

    vtf::DAGNode* node1 = new vtf::DAGNode(1);
    vtf::DAGNode* node2 = new vtf::DAGNode(2);
    vtf::DAGNode* node3 = new vtf::DAGNode(3);
    vtf::DAGNode* node4 = new vtf::DAGNode(4);
    vtf::DAGNode* node5 = new vtf::DAGNode(5);
    vtf::DAGNode* node6 = new vtf::DAGNode(6);


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
    dag.dumpNodeOrder();
}

void testTaskDag()
{
    std::cout << "testTaskDag:\n";

    vtf::Task task1("task_1");
    auto pt1 = task1.commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    vtf::Task task2("task_1");
    auto pt2 = task2.commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    vtf::Task task3("task_1");
    auto pt3 = task3.commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    vtf::Task task4("task_1");
    auto pt4 = task4.commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    vtf::Task task5("task_1");
    auto pt5 = task5.commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);
    task5.setPriority(vtf::TaskPriority::URGENCY);

    vtf::Task task6("task_1");
    auto pt6 = task6.commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    vtf::Task task7("task_1");
    auto pt7 = task7.commit([](int a, int b) {
        std::cout << "exec a + b" << std::endl;
        return a + b;
    }, 1, 2);

    task4.precede(&task3);
    task4.precede(&task6);
    task3.precede(&task6);
    task3.precede(&task2);
    task6.precede(&task5);
    task6.precede(&task1);
    task1.precede(&task2);
    task5.precede(&task7);  
    task1.precede(&task7);
    task2.precede(&task7);


    vtf::DAG dag;
    dag.addNode(&task1);
    dag.addNode(&task2);
    dag.addNode(&task3);
    dag.addNode(&task4);
    dag.addNode(&task5);
    dag.addNode(&task6);
    dag.addNode(&task7);

    dag.buildGraph();
    dag.dumpGraph();
    dag.topologicalSort();
    dag.dumpNodeOrder();

    std::cout << "twitch" << std::endl;
    dag.rebuildGraph();
    dag.dumpGraph();
    dag.topologicalSort();
    dag.dumpNodeOrder();
    

}

int main()
{
    testDAGbasic();
    testTaskDag();
}