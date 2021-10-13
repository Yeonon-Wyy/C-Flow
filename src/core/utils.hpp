/*
 * @Descripttion: include some util
 * @version: v1.0
 * @Author: yeonon
 * @Date: 2021-09-24 19:47:30
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-13 21:50:24
 */
#pragma once
#include <atomic>
#include <type_traits>
#include <sstream>
#include <chrono>

namespace vtf {

namespace util {

class IDGenerator {
public:
    static IDGenerator* getInstance()
    {
        static IDGenerator idGenerator;
        return &idGenerator;
    }
    long generate()
    {
        m_id++;
        return m_id;
    }
public:
    std::atomic_long m_id;
};

class StringConvetor {
public:
    template<typename digitType>
    static const std::string digit2String(digitType digit)
    {
        std::ostringstream oss;
        oss << digit;
        return oss.str();
    }

    template<typename digitType>
    static digitType string2digit(std::string str)
    {
        std::istringstream iss(str);  
        digitType digit;  
        iss >> digit;  
        return digit;      
    }
};

class TimeUtil
{
public:
    static auto awake_time(int timeInMs) {
        // using std::chrono::operator""ms;
        std::chrono::milliseconds ms(timeInMs);
        return std::chrono::steady_clock::now() + ms;
    }

    template<typename T>
    static constexpr T convertTime(std::chrono::duration<double> originTime)
    {
        return std::chrono::duration_cast<T>(originTime);
    }
};

} //namespace util
} //namespace vtf