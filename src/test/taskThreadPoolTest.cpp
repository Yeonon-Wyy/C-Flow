/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-10 20:10:02
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-10 21:21:31
 */
#include "../core/Dag.hpp"
#include "../core/task.hpp"
#include "../core/taskthreadPool.hpp"

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
        std::this_thread::sleep_until(awake_time());
        std::cout << "task1 complate!" << std::endl;
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::Task> task2 = std::make_shared<vtf::Task>("task_2");
    auto pt2 = task2->commit([](int a, int b) {
        std::this_thread::sleep_until(awake_time());
        std::cout << "task2 complate!" << std::endl;
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::Task> task3 = std::make_shared<vtf::Task>("task_3");
    auto pt3 = task3->commit([](int a, int b) {
        std::this_thread::sleep_until(awake_time());
        std::cout << "task3 complate!" << std::endl;
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::Task> task4 = std::make_shared<vtf::Task>("task_4");
    auto pt4 = task4->commit([](int a, int b) {
        std::this_thread::sleep_until(awake_time());
        std::cout << "task4 complate!" << std::endl;
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::Task> task5 = std::make_shared<vtf::Task>("task_5");
    auto pt5 = task5->commit([](int a, int b) {
        std::this_thread::sleep_until(awake_time());
        std::cout << "task5 complate!" << std::endl;
        return a + b;
    }, 1, 2);
    task5->setPriority(vtf::TaskPriority::URGENCY);

    std::shared_ptr<vtf::Task> task6 = std::make_shared<vtf::Task>("task_6");

    auto pt6 = task6->commit([](int a, int b) {
        std::this_thread::sleep_until(awake_time());
        std::cout << "task6 complate!" << std::endl;
        return a + b;
    }, 1, 2);

    std::shared_ptr<vtf::Task> task7 = std::make_shared<vtf::Task>("task_7");
    auto pt7 = task7->commit([](int a, int b) {
        std::this_thread::sleep_until(awake_time());
        std::cout << "task7 complate!" << std::endl;
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
    // task2->precede(task7);


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

    vtf::TaskThreadPool pool(4);


    auto start = std::chrono::system_clock::now();
    std::vector<std::vector<long>> topoOrder = dag.topologicalSort();

    for (auto& level : topoOrder) {
        std::cout << "[";
        for (long taskId : level) {
            std::cout << taskId << ",";
        }
        std::cout << "],";
    }
    std::cout << std::endl;

    for (auto& order : topoOrder) {
        std::vector<std::future<void>> futureList;
        for (long taskId : order) {
            futureList.push_back(pool.emplaceTask(taskMap[taskId]));
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
    testTaskExecute();
    return 0;
}