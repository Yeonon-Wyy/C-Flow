/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-09-25 20:35:55
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-02 15:54:09
 */
#pragma once

#include <vector>
#include <unordered_map>
#include "./utils.hpp"
#include <iostream>

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
    void precede(DAGNode* otherNode);

public:
    long m_nodeId;                    //node-id
    std::vector<long> m_outNodes;      //out-degree
    int m_indegree = 0;               //in-degree of node
    int m_outdegree = 0;              //out-degree of node
};

class DAG {
public:

    /**
     * @name: addNode
     * @Descripttion: add DagNode to Graph
     * @param {DAGNode*} node
     * @return {*}
     */    
    void addNode(DAGNode* node);

    /**
     * @name: buildGraph
     * @Descripttion: use node infos to build a DAG
     * @param {*}
     * @return {*}
     */    
    void buildGraph();

    /**
     * @name: topologicalSort
     * @Descripttion: topological sort algorithem for DAG
     * @param {*}
     * @return {*}
     */    
    void topologicalSort();

    /**
     * @name: dumpGraph
     * @Descripttion: dump Graph info
     * @param {*}
     * @return {*}
     */    
    void dumpGraph();

    /**
     * @name: dump
     * @Descripttion: 
     * @param {*}
     * @return {*}
     */    
    void dumpNodeOrder();

    /**
     * @name: rebuildGraph
     * @Descripttion: rebuild the graph, only use in after topologicalSort.
     * @param {*}
     * @return {*}
     */    
    void rebuildGraph();

private:
    std::unordered_map<long, std::vector<long>> m_graph;
    std::unordered_map<long, DAGNode*> m_nodeIdMap;
    std::vector<long> m_nodeOrder;
    std::unordered_map<long, long> m_nodeIndegreeMap;
    bool m_graphModifiedFlag = false;
};

/*
* class DAGNode
*/


DAGNode::DAGNode(int id)
        :m_nodeId(id)
{
}

void DAGNode::precede(DAGNode* otherNode)
{
    this->m_outNodes.push_back(otherNode->m_nodeId);
    this->m_outdegree++;
    otherNode->m_indegree++;
}

/*
* class DAG
*/
void DAG::addNode(DAGNode* node)
{
    long nodeId = node->m_nodeId;
    m_nodeIdMap[nodeId] = node;
}

void DAG::buildGraph()
{
    for (auto &[nodeId, node] : m_nodeIdMap) {
        if (m_nodeIndegreeMap.count(nodeId) == 0) m_nodeIndegreeMap[nodeId] = 0;
        for (long otherNOdeId : node->m_outNodes) {
            m_graph[nodeId].push_back(otherNOdeId);
            m_nodeIndegreeMap[otherNOdeId]++;
        }
    }
}

void DAG::topologicalSort() {
    rebuildGraph();
    while (true) {
        bool findFlag = false;
        for (auto &[nodeId, degree] : m_nodeIndegreeMap) {
            if (degree == 0) {
                //can find node of degree is 0
                long findNodeId = nodeId;
                for (long otherNodeId : m_graph[findNodeId]) {
                    m_nodeIndegreeMap[otherNodeId]--;
                }
                m_graph.erase(findNodeId);
                m_nodeIndegreeMap.erase(findNodeId);
                m_nodeOrder.push_back(findNodeId);
                findFlag = true;
                break;
            }
        }
        if (!findFlag) {
            if (!m_nodeIndegreeMap.empty())
                std::cout << "error! please check dep" << std::endl;
            m_graphModifiedFlag = true;
            return;
        }
    }
}


void DAG::dumpGraph()
{
    for (auto &[nodeId, outNodeIds] : m_graph) {
        std::cout << nodeId << ": ";
        for (long outNodeId : outNodeIds) {
            std::cout << outNodeId << " ";
        }
        std::cout << std::endl;
    }
}

void DAG::dumpNodeOrder()
{
    for (long nodeId : m_nodeOrder) {
        std::cout << nodeId << " ";
    }
    std::cout << std::endl;
}

void DAG::rebuildGraph()
{
    if (m_graphModifiedFlag == false) return;
    m_graph.clear();
    m_nodeIndegreeMap.clear();
    m_nodeOrder.clear();
    buildGraph();
    m_graphModifiedFlag = false;
}

}