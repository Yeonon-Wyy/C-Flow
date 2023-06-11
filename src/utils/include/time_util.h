/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-12-03 23:03:20
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-09-04 19:24:43
 */
#pragma once

#include <chrono>

namespace cflow::utils {
class TimeUtil
{
public:
    /**
     * @name: awake_time
     * @Descripttion: return a chronon clock object with time in ms
     * @param {int} timeInMs
     * @return {*} chronon clock object
     */
    static auto awake_time(int timeInMs)
    {
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
    template <typename T>
    static constexpr T convertTime(std::chrono::duration<double> originTime)
    {
        return std::chrono::duration_cast<T>(originTime);
    }
};

} // namespace cflow::utils
