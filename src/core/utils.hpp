/*
 * @Descripttion: include some util
 * @version: v1.0
 * @Author: yeonon
 * @Date: 2021-09-24 19:47:30
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-30 18:58:48
 */
#pragma once
#include <atomic>
#include <type_traits>
#include <sstream>
#include <chrono>

#include "log.hpp"

namespace vtf {

namespace util {

class IDGenerator {
public:
    static IDGenerator* getInstance()
    {
        static IDGenerator idGenerator;
        return &idGenerator;
    }

    /**
     * @name: generate
     * @Descripttion: generate a id for user
     * @param {*}
     * @return {*} id
     */    
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

    /**
     * @name: digit2String
     * @Descripttion: convert digit to str
     * @param {*} digit
     * @return {*} str
     */    
    template<typename digitType>
    static const std::string digit2String(digitType digit)
    {
        std::ostringstream oss;
        oss << digit;
        return oss.str();
    }

    /**
     * @name: string2digit
     * @Descripttion: convert str to digit
     * @param {*} str
     * @return {*} digit
     */    
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

    /**
     * @name: awake_time
     * @Descripttion: return a chronon clock object with time in ms
     * @param {int} timeInMs
     * @return {*} chronon clock object
     */    
    static auto awake_time(int timeInMs) {
        // using std::chrono::operator""ms;
        std::chrono::milliseconds ms(timeInMs);
        return std::chrono::steady_clock::now() + ms;
    }

    /**
     * @name: convertTime
     * @Descripttion: convert a duration to another duration, like ms to ns
     * @param {*} originTime
     * @return {*} after convert time
     */    
    template<typename T>
    static constexpr T convertTime(std::chrono::duration<double> originTime)
    {
        return std::chrono::duration_cast<T>(originTime);
    }
};

} //namespace util
} //namespace vtf