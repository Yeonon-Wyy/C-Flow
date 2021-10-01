/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-09-25 20:35:55
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-01 19:53:34
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
    std::vector<int> m_outNodes;      //out-degree
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
private:
    std::unordered_map<int, std::vector<int>> graph;
    std::unordered_map<int, DAGNode*> nodeIdMap;
    std::vector<int> m_nodeOrder;
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
    int nodeId = node->m_nodeId;
    nodeIdMap[nodeId] = node;
}

void DAG::buildGraph()
{
    for (auto &[nodeId, node] : nodeIdMap) {
        for (int otherNOdeId : node->m_outNodes) {
            graph[nodeId].push_back(otherNOdeId);
        }
    }
}

void DAG::topologicalSort() {
    while (true) {
        bool findFlag = false;
        for (auto &[nodeId, node] : nodeIdMap) {
            if (node->m_indegree == 0) {
                int findNodeId = nodeId;
                //find
                for (int otherNodeId : graph[findNodeId]) {
                    nodeIdMap[otherNodeId]->m_indegree--;
                }
                graph.erase(findNodeId);
                nodeIdMap.erase(findNodeId);
                m_nodeOrder.push_back(findNodeId);
                findFlag = true;
                break;
            }
        }
        if (!findFlag) {
            if (!nodeIdMap.empty())
                std::cout << "error! please check dep" << std::endl;
            return;
        }
    }
}


void DAG::dumpGraph()
{
    for (auto &[nodeId, outNodeIds] : graph) {
        std::cout << nodeId << ": ";
        for (int outNodeId : outNodeIds) {
            std::cout << outNodeId << " ";
        }
        std::cout << std::endl;
    }
}

void DAG::dumpOrder()
{
    for (int nodeId : m_nodeOrder) {
        std::cout << nodeId << " ";
    }
    std::cout << std::endl;
}

}