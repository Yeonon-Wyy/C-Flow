/*
 * @Descripttion: include some util
 * @version: v1.0
 * @Author: yeonon
 * @Date: 2021-09-24 19:47:30
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-02 17:37:14
 */
#pragma once
#include <atomic>

namespace vtf {

class IDGenerator {
public:
    static IDGenerator* getInstance();
    long generate();
public:
    std::atomic_long m_id;
};

IDGenerator* IDGenerator::getInstance()
{
    static IDGenerator idGenerator;
    return &idGenerator;
}

long IDGenerator::generate() 
{
    m_id++;
    return m_id;
}

} //namespace vtf