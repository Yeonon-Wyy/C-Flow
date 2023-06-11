#include "dumper.h"

namespace cflow::utils {
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
    if (m_dumpType == DUMPTYPE::PIPELINE_ALL ||
        m_dumpType == DUMPTYPE::PIPELINE_EACH)
    {
        addEdgeForPipeline(output);
    }
    else if (m_dumpType == DUMPTYPE::TASKFLOW)
    {
        addEdgeForTaskFlow(output);
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

void Dumper::addEdgeForTaskFlow(std::ostream& output)
{
    // [[1],[2,3],[4]]
    std::map<ScenarioType, std::vector<std::string>> taskOrders(
        m_pipelines.begin(), m_pipelines.end());
    auto preIt = taskOrders.begin();
    auto curIt = std::next(taskOrders.begin(), 1);
    while (curIt != taskOrders.end())
    {
        auto preTasks = preIt->second;
        auto curTasks = curIt->second;
        for (auto&& preTask : preTasks)
        {
            for (auto&& curTask : curTasks)
            {
                addDOTEdge(output, preTask, curTask, "");
            }
        }
        preIt = curIt;
        curIt = std::next(curIt, 1);
        if (curIt == taskOrders.end())
        {
            for (auto&& curTask : curTasks)
            {
                addDOTEdge(output, curTask, "End", "");
            }
        }
    }
}

void Dumper::addEdgeForPipeline(std::ostream& output)
{
    for (auto&& [curScenario, pipeline] : m_pipelines)
    {
        pipeline.push_back("End");
        for (size_t i = 1; i < pipeline.size(); i++)
        {
            std::string edgeLabel = " ";
            if (m_dumpType == DUMPTYPE::PIPELINE_ALL)
            {
                edgeLabel = "Scenario_" + std::to_string(curScenario);
            }
            addDOTEdge(output, pipeline[i - 1], pipeline[i], edgeLabel);
        }
    }
}

} // namespace cflow::utils