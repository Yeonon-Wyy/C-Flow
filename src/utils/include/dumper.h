/*
 * @Author: Yeonon
 * @Date: 2022-10-30 15:51:28
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-11-06 22:03:37
 * @FilePath: /src/core/utils/dumper.h
 * @Description:
 * Copyright 2022 Yeonon, All Rights Reserved.
 * 2022-10-30 15:51:28
 */
#pragma once

#include <string>
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <iterator>
#include <map>
#include <set>
#include <vector>

enum class DUMPTYPE
{
    PIPELINE_EACH,
    PIPELINE_ALL,
    TASKFLOW,
};

namespace cflow::utils {

class Dumper
{
private:
    using ScenarioType = uint32_t;
    using PipelineType =
        std::unordered_map<ScenarioType, std::vector<std::string>>;

public:
    Dumper(const std::string& graphName, PipelineType& pipelines,
           DUMPTYPE dumpType = DUMPTYPE::PIPELINE_EACH);

private:
    Dumper(Dumper&&)      = delete;
    Dumper(const Dumper&) = delete;
    Dumper& operator=(const Dumper&) = delete;

public:
    void dumpDOT(std::ostream& ostream);

private:
    void addDOTNode(std::ostream& output, const std::string& nodeName);
    void addDOTEdge(std::ostream& output, const std::string& srcNode,
                    const std::string& dstNode, const std::string& label);

    void addEdgeForTaskFlow(std::ostream& output);
    void addEdgeForPipeline(std::ostream& output);

private:
    std::string  m_graphName;
    PipelineType m_pipelines;
    DUMPTYPE     m_dumpType;
};

} // namespace cflow::utils
