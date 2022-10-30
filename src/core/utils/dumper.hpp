/*
 * @Author: Yeonon
 * @Date: 2022-10-30 15:51:28
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-10-30 21:01:30
 * @FilePath: /src/core/utils/dumper.hpp
 * @Description:
 * Copyright 2022 Yeonon, All Rights Reserved.
 * 2022-10-30 15:51:28
 */
#pragma once

#include <string>
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include "../type.hpp"

namespace cflow::utils {

class Dumper
{
private:
    using ScenarioType = uint32_t;
    using PipelineType =
        std::unordered_map<ScenarioType, std::vector<std::string>>;

public:
    Dumper(const std::string& graphName, PipelineType& pipelines,
           DUMPTYPE dumpType = DUMPTYPE::EACH);

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

private:
    std::string  m_graphName;
    PipelineType m_pipelines;
    DUMPTYPE     m_dumpType;
};

Dumper::Dumper(const std::string& graphName, PipelineType& nodeNames,
               DUMPTYPE dumpType)
    : m_graphName(graphName),
      m_pipelines(nodeNames),
      m_dumpType(dumpType)
{
}

void Dumper::dumpDOT(std::ostream& output)
{
    output << "digraph " << m_graphName << " {\n";
    output << "\tlabel=\"" << m_graphName << "\"\n";
    output << "\trankdir=LR\n";

    std::unordered_set<std::string> pipelineNodeNames;

    for (auto&& [curScenario, pipeline] : m_pipelines)
    {
        for (auto&& nodeName : pipeline)
        {
            pipelineNodeNames.insert(nodeName);
        }
    }

    // add node
    for (auto&& nodeName : pipelineNodeNames)
    {
        addDOTNode(output, nodeName);
    }

    // add dummy end node
    addDOTNode(output, "End");

    // add edge
    for (auto&& [curScenario, pipeline] : m_pipelines)
    {
        pipeline.push_back("End");
        for (int i = 1; i < pipeline.size(); i++)
        {
            std::string edgeLabel = " ";
            if (m_dumpType == DUMPTYPE::ALL)
            {
                edgeLabel = "Scenario_" + std::to_string(curScenario);
            }
            addDOTEdge(output, pipeline[i - 1], pipeline[i], edgeLabel);
        }
    }

    output << "}\n";
}

void Dumper::addDOTNode(std::ostream& output, const std::string& nodeName)
{
    output << "\t" << nodeName << "["
           << "label=\"" << nodeName << "\",shape=\"box\""
           << "]\n";
}

void Dumper::addDOTEdge(std::ostream& output, const std::string& srcNode,
                        const std::string& dstNode, const std::string& label)
{
    output << "\t" << srcNode << "->" << dstNode << "[label=\"" << label
           << "\"]\n";
}

} // namespace cflow::utils
