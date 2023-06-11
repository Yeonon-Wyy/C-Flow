#include "dag.h"

namespace cflow {
/*
 * class DAGNode
 */
DAGNode::DAGNode(cflow_id_t id) : m_nodeId(id) {}

void DAGNode::connect(std::shared_ptr<DAGNode> succsorNode)
{
    CFLOW_LOGD("cur node id {0}, next node id {1}", this->m_nodeId,
               succsorNode->m_nodeId);
    this->m_outNodes.push_back(succsorNode->m_nodeId);
}

/*
 * class DAG
 */
void DAG::addNode(std::shared_ptr<DAGNode> node)
{
    cflow_id_t nodeId   = node->getNodeId();
    m_nodeIdMap[nodeId] = node;

    // set flag for grpah is modified
    m_graphModified = true;
}

void DAG::buildGraph()
{
    for (auto& [nodeId, node] : m_nodeIdMap)
    {
        auto nodeSp = node.lock();
        for (cflow_id_t otherNOdeId : nodeSp->getOutNodes())
        {
            m_graph[nodeId].push_back(otherNOdeId);
            if (m_nodeIndegreeMap.count(nodeId) == 0)
            {
                m_nodeIndegreeMap[nodeId] = 0;
            }
            m_nodeIndegreeMap[otherNOdeId]++;
        }
    }
}

std::vector<std::vector<cflow_id_t>> DAG::topologicalSort()
{
    if (!m_graphModified) return m_nodeOrderCache;
    rebuildGraphIfNeed();

    std::vector<std::vector<cflow_id_t>> nodeOrder;
    std::vector<cflow_id_t>              sameLevelNodes;

    std::unordered_map<cflow_id_t, std::vector<cflow_id_t>> graphCopy = m_graph;
    std::unordered_map<cflow_id_t, cflow_id_t> nodeIndegreeMapCopy =
        m_nodeIndegreeMap;

    while (true)
    {
        for (auto& [nodeId, degree] : nodeIndegreeMapCopy)
        {
            if (degree == 0)
            {
                sameLevelNodes.push_back(nodeId);
            }
        }

        if (!sameLevelNodes.empty())
        {
            for (int nodeId : sameLevelNodes)
            {
                // erase node info from graph
                for (cflow_id_t otherNodeId : graphCopy[nodeId])
                {
                    nodeIndegreeMapCopy[otherNodeId]--;
                }
                graphCopy.erase(nodeId);
                nodeIndegreeMapCopy.erase(nodeId);
            }
            nodeOrder.push_back(sameLevelNodes);
            sameLevelNodes.clear();
        }
        else
        {
            // no find node, maybe error or complete
            if (!nodeIndegreeMapCopy.empty())
            {
                CFLOW_LOGE("please check node dependcy.");
                // we must clear error node order
                nodeOrder.clear();
            }
            break;
        }
    }
    m_nodeOrderCache = nodeOrder;
    return nodeOrder;
}

std::set<std::vector<cflow_id_t>> DAG::multiTopologicalSort()
{
    rebuildGraphIfNeed();
    std::vector<std::vector<cflow_id_t>> nodeOrder;
    std::vector<cflow_id_t>              sameLevelNodes;

    std::unordered_map<cflow_id_t, std::vector<cflow_id_t>> graphCopy = m_graph;
    std::unordered_map<cflow_id_t, cflow_id_t> nodeIndegreeMapCopy =
        m_nodeIndegreeMap;
    while (true)
    {
        for (auto& [nodeId, degree] : nodeIndegreeMapCopy)
        {
            if (degree == 0)
            {
                sameLevelNodes.push_back(nodeId);
            }
        }

        if (!sameLevelNodes.empty())
        {
            for (int nodeId : sameLevelNodes)
            {
                // erase node info from graph
                for (cflow_id_t otherNodeId : graphCopy[nodeId])
                {
                    nodeIndegreeMapCopy[otherNodeId]--;
                }
                graphCopy.erase(nodeId);
                nodeIndegreeMapCopy.erase(nodeId);
            }
            nodeOrder.push_back(sameLevelNodes);
            sameLevelNodes.clear();
        }
        else
        {
            // no find node, maybe error or complete
            if (!nodeIndegreeMapCopy.empty())
            {
                CFLOW_LOGD("please check node dependcy.");
                // we must clear error node order
                nodeOrder.clear();
            }
            break;
        }
    }
    std::set<std::vector<cflow_id_t>> allTopoOrder;
    std::vector<cflow_id_t>           topoOrder;
    findAllTopologicalSort(nodeOrder, allTopoOrder, topoOrder, 0);
    return allTopoOrder;
}

void DAG::findAllTopologicalSort(
    std::vector<std::vector<cflow_id_t>>& nodeOrder,
    std::set<std::vector<cflow_id_t>>&    allTopoOrder,
    std::vector<cflow_id_t>& topoOrder, int startIdx)
{
    if (topoOrder.size() == nodeOrder.size())
    {
        std::vector<cflow_id_t> temp;
        for (size_t i = 0; i < topoOrder.size(); i++)
        {
            if (i > 0 && !checkDependency(topoOrder[i - 1], topoOrder[i]))
            {
                break;
            }
            temp.push_back(topoOrder[i]);
        }
        if (temp.size() > 1) allTopoOrder.insert(temp);
        // allTopoOrder.insert(topoOrder);
        return;
    }

    for (size_t i = startIdx; i < nodeOrder.size(); i++)
    {
        for (size_t j = 0; j < nodeOrder[i].size(); j++)
        {
            topoOrder.push_back(nodeOrder[i][j]);
            findAllTopologicalSort(nodeOrder, allTopoOrder, topoOrder, i + 1);
            topoOrder.pop_back();
        }
    }
}

bool DAG::checkDependency(cflow_id_t srcNodeId, cflow_id_t dstNodeId)
{
    if (m_graph.count(srcNodeId) == 0)
    {
        return false;
    }

    auto connectionsIter = m_graph.find(srcNodeId);
    auto resultIter      = std::find(connectionsIter->second.cbegin(),
                                connectionsIter->second.cend(), dstNodeId);
    if (resultIter == connectionsIter->second.cend())
    {
        return false;
    }
    return true;
}

void DAG::dumpGraph()
{
    for (auto& [nodeId, outNodeIds] : m_graph)
    {
        std::stringstream ss;
        ss << nodeId << ": ";
        for (cflow_id_t outNodeId : outNodeIds)
        {
            ss << outNodeId << " ";
        }
        CFLOW_LOGD(ss.str());
    }
}

void DAG::rebuildGraphIfNeed()
{
    if (m_graphModified == false) return;
    m_graph.clear();
    m_nodeIndegreeMap.clear();
    buildGraph();
    m_graphModified = false;
}

void DAG::clear()
{
    m_graph.clear();
    m_graphModified = true;
    m_nodeIdMap.clear();
    m_nodeIndegreeMap.clear();
    m_nodeOrderCache.clear();
}
} // namespace cflow