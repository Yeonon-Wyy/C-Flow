/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-11-14 23:36:59
 * @LastEditors: yeonon
 * @LastEditTime: 2021-11-15 22:38:47
 */
#pragma once

#include "../notifier.hpp"
#include "../log.hpp"

namespace vtf {
namespace pipeline {

constexpr int defaultReadyQueueSize = 32;


template<typename Item>
class ResultNotifier : public Notifier<Item> {
public:

    ResultNotifier(const std::string& name, int readyQueueSize = defaultReadyQueueSize)
        :Notifier<Item>(name, 32)
    {}
    bool process(std::shared_ptr<Item>) override;
};

template<typename Item>
bool ResultNotifier<Item>::process(std::shared_ptr<Item> item)
{
    return true;
}

}
}