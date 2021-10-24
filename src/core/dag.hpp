/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-09-25 20:35:55
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-24 13:36:09
 */
#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <memory>

#include "./utils.hpp"
#include "log.hpp"

namespace vtf
{

class DAGNode {

public:

    /**
     * @name: DAGNode
     * @Descripttion: constructor of DAGNode
     * @param {int} id is node id
     * @return {*}
     */    
    DAGNode(int id);

    /**
     * @name: precede
     * @Descripttion: precede set dependency of node, nodeA->precede(NodeB) mean A -> B
     * @param {DAGNode*} otherNode
     * @return {*}
     */    
    void precede(std::shared_ptr<DAGNode> otherNode);


    /**
     * @name: getNodeId
     * @Descripttion: just get node id
     * @param {*}
     * @return {*} node-id
     */    
    long getNodeId() { return m_nodeId; }

    /**
     * @name: getOutNodes
     * @Descripttion: just return outNodes, in c++ 17, it can use copy-elison
     * @param {*}
     * @return {*}
     */    
    std::vector<long> getOutNodes() { return m_outNodes; }

private:
    long m_nodeId;                     //node-id
    std::vector<long> m_outNodes;      //out-degree
};

class DAG {
public:

    /**
     * @name: addNode
     * @Descripttion: add DagNode to Graph
     * @param {DAGNode*} node
     * @return {*}
     */    
    void addNode(std::shared_ptr<DAGNode> node);

    /**
     * @name: buildGraph
     * @Descripttion: use node infos to build a DAG
     * @param {*}
     * @return {*}
     */    
    void buildGraph();


    /**
     * @name: rebuildGraphIfNeed
     * @Descripttion: rebuild the graph, only use in after topologicalSort.
     * @param {*}
     * @return {*}
     */    
    void rebuildGraphIfNeed();

    /**
     * @name: topologicalSort
     * @Descripttion: topological sort algorithem for DAG
     * @param {*}
     * @return {*} topological order
     */    
    std::vector<std::vector<long>> topologicalSort();

    /**
     * @name: dumpGraph
     * @Descripttion: dump Graph info
     * @param {*}
     * @return {*}
     */    
    void dumpGraph();

private:
    std::unordered_map<long, std::vector<long>> m_graph;
    std::unordered_map<long, std::shared_ptr<DAGNode>> m_nodeIdMap;
    std::unordered_map<long, long> m_nodeIndegreeMap;
    bool m_graphModified = false;
    std::vector<std::vector<long>> m_nodeOrderCache;
};

/*
* class DAGNode
*/
DAGNode::DAGNode(int id)
        :m_nodeId(id)
{
}


void DAGNode::precede(std::shared_ptr<DAGNode> otherNode)
{
    this->m_outNodes.push_back(otherNode->m_nodeId);
}

/*
* class DAG
*/
void DAG::addNode(std::shared_ptr<DAGNode> node)
{
    long nodeId = node->getNodeId();
    m_nodeIdMap[nodeId] = node;
    
    //set flag for grpah is modified
    m_graphModified = true;
}

void DAG::buildGraph()
{
    for (auto &[nodeId, node] : m_nodeIdMap) {
        if (m_nodeIndegreeMap.count(nodeId) == 0) m_nodeIndegreeMap[nodeId] = 0;
        for (long otherNOdeId : node->getOutNodes()) {
            m_graph[nodeId].push_back(otherNOdeId);
            m_nodeIndegreeMap[otherNOdeId]++;
        }
    }
}


std::vector<std::vector<long>> DAG::topologicalSort() {
    if (!m_graphModified) return m_nodeOrderCache;
    rebuildGraphIfNeed();

    std::vector<std::vector<long>> nodeOrder;
    std::vector<long> sameLevelNodes;

    std::unordered_map<long, std::vector<long>> graphCopy = m_graph;
    std::unordered_map<long, long> nodeIndegreeMapCopy = m_nodeIndegreeMap;

    while (true) {
        for (auto &[nodeId, degree] : nodeIndegreeMapCopy) {
            if (degree == 0) {
                sameLevelNodes.push_back(nodeId);
            }
        }

        if (!sameLevelNodes.empty()) {
            for (int nodeId : sameLevelNodes) {
            //erase node info from graph 
                for (long otherNodeId : graphCopy[nodeId]) {
                    nodeIndegreeMapCopy[otherNodeId]--;
                }
                graphCopy.erase(nodeId);
                nodeIndegreeMapCopy.erase(nodeId);
            }
            nodeOrder.push_back(sameLevelNodes);
            sameLevelNodes.clear();
        } else {
            //no find node, maybe error or complete
            if (!nodeIndegreeMapCopy.empty()) {
                VTF_LOGD("please check node dependcy.");
                //we must clear error node order
                nodeOrder.clear();
            }
            break;
        }
    }
    m_nodeOrderCache = nodeOrder;
    return nodeOrder;
}


void DAG::dumpGraph()
{
    for (auto &[nodeId, outNodeIds] : m_graph) {
        std::stringstream ss;
        ss << nodeId << ": ";
        for (long outNodeId : outNodeIds) {
            ss << outNodeId << " ";
        }
        VTF_LOGI(ss.str());
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

}