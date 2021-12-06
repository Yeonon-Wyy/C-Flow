/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-12-03 23:04:22
 * @LastEditors: yeonon
 * @LastEditTime: 2021-12-03 23:06:25
 */
#pragma once

#include <atomic>

namespace vtf {
namespace utils {
class IDGenerator {
public:
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
private:
    std::atomic_long m_id;
};
}
}