/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-12-03 23:04:22
 * @LastEditors: yeonon
 * @LastEditTime: 2022-01-29 14:46:14
 */
#pragma once

#include <atomic>
#include "../type.hpp"

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
    vtf_id_t generate()
    {
        m_id++;
        return m_id;
    }
private:
    std::atomic_long m_id;
};
}
}