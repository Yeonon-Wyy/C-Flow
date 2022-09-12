/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-12-03 23:05:33
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-09-04 19:24:32
 */
#pragma once

#include <sstream>
#include <type_traits>

namespace cflow::utils {
class StringConvetor
{
public:
    /**
     * @name: digit2String
     * @Descripttion: convert digit to str
     * @param {*} digit
     * @return {*} str
     */
    template <typename digitType>
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
    template <typename digitType>
    static digitType string2digit(std::string str)
    {
        std::istringstream iss(str);
        digitType digit;
        iss >> digit;
        return digit;
    }
};

} // namespace cflow::utils