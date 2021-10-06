/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-09-25 20:35:55
 * @LastEditors: yeonon
 * @LastEditTime: 2021-10-06 18:23:33
 */
#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>
#include <algorithm>

#include "./utils.hpp"

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
     * @name: DAGNode
     * @Descripttion: constructor of DAGNode
     * @param {int} id is node id
     * @return {*}
     */    
    DAGNode(int id, int nodePriority);

    /**
     * @name: precede
     * @Descripttion: precede set dependency of node, nodeA->precede(NodeB) mean A -> B
     * @param {DAGNode*} otherNode
     * @return {*}
     */    
    void precede(DAGNode* otherNode);

    /**
     * @name: getPriority
     * @Descripttion: just return node priority
     * @param {*}
     * @return {*} node priority
     */    
    int getPriority() { return m_nodePriority; }

    /**
     * @name: setPriority
     * @Descripttion: just set node priority
     * @param {int} nodePriority
     * @return {*}
     */    
    void setPriority(int nodePriority) { m_nodePriority = nodePriority; }

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
    int m_nodePriority;                //node priority
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
     * @name: rebuildGraph
     * @Descripttion: rebuild the graph, only use in after topologicalSort.
     * @param {*}
     * @return {*}
     */    
    void rebuildGraph();

    /**
     * @name: topologicalSort
     * @Descripttion: topological sort algorithem for DAG
     * @param {*}
     * @return {*}
     */    
    void topologicalSort();

    std::vector<std::vector<long>> getTopoOrder() { return m_nodeTopoOrder; }

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
    std::unordered_map<long, std::vector<long>> m_graph;
    std::unordered_map<long, DAGNode*> m_nodeIdMap;
    std::unordered_map<long, long> m_nodeIndegreeMap;
    bool m_graphModifiedFlag = false;
    std::vector<std::vector<long>> m_nodeTopoOrder;
};

/*
* class DAGNode
*/
DAGNode::DAGNode(int id)
        :m_nodeId(id),
         m_nodePriority(0)
{
}

DAGNode::DAGNode(int id, int nodePriority)
        :m_nodeId(id),
         m_nodePriority(nodePriority)
{

}

void DAGNode::precede(DAGNode* otherNode)
{
    this->m_outNodes.push_back(otherNode->m_nodeId);
}

/*
* class DAG
*/
void DAG::addNode(DAGNode* node)
{
    long nodeId = node->getNodeId();
    m_nodeIdMap[nodeId] = node;
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

// topologicalSort V1, just comment it first

// void DAG::topologicalSort() {
//     rebuildGraph();
//     while (true) {
//         long findNodeId = -1;
//         int curMaxPriority = INT32_MIN;
//         for (auto &[nodeId, degree] : m_nodeIndegreeMap) {
//             if (degree == 0) {
//                 //can find node of degree is 0
//                 if (m_nodeIdMap[nodeId]->getPriority() > curMaxPriority) {
//                     findNodeId = nodeId;
//                     curMaxPriority = m_nodeIdMap[nodeId]->getPriority();
//                 }
//             }
//         }

//         if (findNodeId != -1) {
//             //erase node info from graph 
//             for (long otherNodeId : m_graph[findNodeId]) {
//                 m_nodeIndegreeMap[otherNodeId]--;
//             }
//             m_graph.erase(findNodeId);
//             m_nodeIndegreeMap.erase(findNodeId);

//             m_nodeOrder.push_back(findNodeId);
//         } else {
//             //no find node, maybe error or complete
//             if (!m_nodeIndegreeMap.empty())
//                 std::cout << "error! please check dep" << std::endl;
//             m_graphModifiedFlag = true;
//             return;
//         }
//     }
// }

void DAG::topologicalSort() {
    rebuildGraph();
    std::vector<long> sameLevelNodes;
    while (true) {
        for (auto &[nodeId, degree] : m_nodeIndegreeMap) {
            if (degree == 0) {
                sameLevelNodes.push_back(nodeId);
            }
        }

        if (!sameLevelNodes.empty()) {
            for (int nodeId : sameLevelNodes) {
            //erase node info from graph 
                for (long otherNodeId : m_graph[nodeId]) {
                    m_nodeIndegreeMap[otherNodeId]--;
                }
                m_graph.erase(nodeId);
                m_nodeIndegreeMap.erase(nodeId);
            }
            //even some node as same level, still need keep high priority
            std::sort(sameLevelNodes.begin(), sameLevelNodes.end(), [this](int lhs, int rhs) {
                return this->m_nodeIdMap[lhs]->getPriority() > this->m_nodeIdMap[rhs]->getPriority();
            });
            m_nodeTopoOrder.push_back(sameLevelNodes);
            sameLevelNodes.clear();
        } else {
            //no find node, maybe error or complete
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
    for (auto& order : m_nodeTopoOrder) {
        std::cout << "[";
        for (long nodeId : order) {
            std::cout << nodeId << " ";
        }
        std::cout << "] ";
    }
    std::cout << std::endl;
}

void DAG::rebuildGraph()
{
    if (m_graphModifiedFlag == false) return;
    m_graph.clear();
    m_nodeIndegreeMap.clear();
    m_nodeTopoOrder.clear();
    buildGraph();
    m_graphModifiedFlag = false;
}

}