/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-02 18:15:32
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-09 22:35:54
 */

#pragma once

#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>

#include "task.hpp"

namespace vtf
{

class TaskFlowCtl
{
public:
    using runable = std::function<void()>;

    std::shared_ptr<Task> addTask(runable r);
    
    template<typename Function, typename... Args>
    std::shared_ptr<Task> addTask(Function&& f, Args&&... args);

    void start();

private:
    void reorganizeTaskOrder();
    void reorganizeTaskOrderWithPriority();


private:
    std::unordered_map<long, std::shared_ptr<Task>> m_taskIdMap;
    std::vector<std::vector<long>> m_taskOrder;
};



} //namespace vtf