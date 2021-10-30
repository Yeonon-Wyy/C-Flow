/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-24 16:17:33
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-30 18:51:45
 */
/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-10-24 16:17:33
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-30 18:38:15
 */
#pragma once
#include "../dag.hpp"
#include "../threadLoop.hpp"
#include "../log.hpp"
#include "../utils.hpp"

#include <mutex>
#include <memory>
#include <condition_variable>
#include <unordered_map>

namespace vtf {
namespace pipeline {

#define PIPENODE_DEFAULT_NAME_PREFIX "pipeNode_"

template<typename Item>
class PipeNode : 
    public vtf::DAGNode,
    public std::enable_shared_from_this<PipeNode<Item>>
{
public:
    PipeNode()
        :DAGNode(util::IDGenerator::getInstance()->generate()),
         m_id(getNodeId())
    {
        m_name = PIPENODE_DEFAULT_NAME_PREFIX + vtf::util::StringConvetor::digit2String(m_id);
    }

    PipeNode(std::string&& name)
        :DAGNode(util::IDGenerator::getInstance()->generate()),
         m_id(getNodeId()),
         m_name(std::move(name))
    {}

    
    virtual bool process(std::shared_ptr<Item> item) = 0;

    const std::string name() const { return m_name; }

private:
    long m_id;
    std::string m_name;
    bool m_isComplete = false;
};

} //namespace pipeline
} //namespace vtf