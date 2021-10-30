/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-30 17:45:25
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-30 17:54:26
 */
#pragma once

#include "../utils.hpp"
namespace vtf {
namespace pipeline {

class Request {

public:
    Request()
        :m_id(vtf::util::IDGenerator::getInstance()->generate())
    {}

    long ID() { return m_id; };
private:
    long m_id;
};

class PipeRequest : public Request {
public:
    PipeRequest()
        :Request()
    {}
};

} //namespace pipeline
} //namespace vtf