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