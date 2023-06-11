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

namespace cflow::utils {
class IDGenerator
{
public:
    IDGenerator();
    /**
     * @name: generate
     * @Descripttion: generate a id for user
     * @param {*}
     * @return {*} id
     */
    long generate();

private:
    std::atomic_long m_id;
    std::mutex       m_idLock;
};

} // namespace cflow::utils