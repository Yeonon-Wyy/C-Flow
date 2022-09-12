/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-12-03 23:04:22
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-09-04 19:24:19
 */
#pragma once

#include <atomic>
#include <mutex>

#include "../type.hpp"

namespace cflow::utils {
class IDGenerator
{
public:
    /**
     * @name: generate
     * @Descripttion: generate a id for user
     * @param {*}
     * @return {*} id
     */
    cflow_id_t generate()
    {
        std::unique_lock<std::mutex> lk(m_idLock);
        m_id++;
        return m_id;
    }

private:
    std::atomic_long m_id;
    std::mutex m_idLock;
};

} // namespace cflow::utils