/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2021-09-25 20:35:55
 * @LastEditors: yeonon
 * @LastEditTime: 2022-01-29 14:59:21
 */
#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <memory>
#include <sstream>
#include <unordered_set>
#include <set>
#include "type.hpp"

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
    DAGNode(vtf_id_t id);

    /**
     * @name: connect
     * @Descripttion: connect set dependency of node, nodeA->connect(NodeB) mean A -> B
     * @param {DAGNode*} otherNode
     * @return {*}
     */    
    virtual void connect(std::shared_ptr<DAGNode> otherNode);


    /**
     * @name: getNodeId
     * @Descripttion: just get node id
     * @param {*}
     * @return {*} node-id
     */    
    vtf_id_t getNodeId() { return m_nodeId; }

    /**
     * @name: getOutNodes
     * @Descripttion: just return outNodes, in c++ 17, it can use copy-elison
     * @param {*}
     * @return {*}
     */    
    std::vector<vtf_id_t> getOutNodes() { return m_outNodes; }

private:
    vtf_id_t m_nodeId;                     //node-id
    std::vector<vtf_id_t> m_outNodes;      //out-degree
};

class DAG {
public:
    ~DAG()
    {
        VTF_LOGD("dag destory");
    }

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
    std::vector<std::vector<vtf_id_t>> topologicalSort();

    /**
     * @name: multiTopologicalSort
     * @Descripttion: topological sort algorithem for DAG, will generte multi order
     * @param {*}
     * @return {*} all topological order
     */    
    std::set<std::vector<vtf_id_t>> multiTopologicalSort();


    /**
     * @name: graph
     * @Descripttion: return a graph 
     * @param {*}
     * @return {*}
     */    
    std::unordered_map<vtf_id_t, std::vector<vtf_id_t>> graph() { return m_graph; }
    /**
     * @name: dumpGraph
     * @Descripttion: dump Graph info
     * @param {*}
     * @return {*}
     */    
    void dumpGraph();

    /**
     * @name: clear 
     * @Descripttion: clear all info of dag, include graph info, node info, etc...
     * @param {*}
     * @return {*}
     */    
    void clear();

private:
    /**
     * @name: findAllTopologicalSort
     * @Descripttion: find all topological order by node order
     * @param {*}
     * @return {*}
     */    
    void findAllTopologicalSort(
        std::vector<std::vector<vtf_id_t>>& nodeOrder, 
        std::set<std::vector<vtf_id_t>>& allTopoOrder, 
        std::vector<vtf_id_t>& topoOrder,
        int startIdx
    );

    /**
     * @name: checkDependency
     * @Descripttion: check Dependency with src node id and dst node id
     * @param {vtf_id_t} srcNodeId
     * @param {vtf_id_t} dstNodeId
     * @return {*}
     */    
    bool checkDependency(vtf_id_t srcNodeId, vtf_id_t dstNodeId);

private:
    std::unordered_map<vtf_id_t, std::vector<vtf_id_t>> m_graph;
    std::unordered_map<vtf_id_t, std::weak_ptr<DAGNode>> m_nodeIdMap;
    std::unordered_map<vtf_id_t, vtf_id_t> m_nodeIndegreeMap;
    bool m_graphModified = false;
    std::vector<std::vector<vtf_id_t>> m_nodeOrderCache;
};

/*
* class DAGNode
*/
DAGNode::DAGNode(vtf_id_t id)
        :m_nodeId(id)
{
}


void DAGNode::connect(std::shared_ptr<DAGNode> succsorNode)
{
    this->m_outNodes.push_back(succsorNode->m_nodeId);
}

/*
* class DAG
*/
void DAG::addNode(std::shared_ptr<DAGNode> node)
{
    vtf_id_t nodeId = node->getNodeId();
    m_nodeIdMap[nodeId] = node;
    
    //set flag for grpah is modified
    m_graphModified = true;
}

void DAG::buildGraph()
{
    for (auto &[nodeId, node] : m_nodeIdMap) {
        auto nodeSp = node.lock();
        for (vtf_id_t otherNOdeId : nodeSp->getOutNodes()) {
            m_graph[nodeId].push_back(otherNOdeId);
            if (m_nodeIndegreeMap.count(nodeId) == 0) {
                m_nodeIndegreeMap[nodeId] = 0;
            }
            m_nodeIndegreeMap[otherNOdeId]++;
        }
    }
}


std::vector<std::vector<vtf_id_t>> DAG::topologicalSort() {
    if (!m_graphModified) return m_nodeOrderCache;
    rebuildGraphIfNeed();

    std::vector<std::vector<vtf_id_t>> nodeOrder;
    std::vector<vtf_id_t> sameLevelNodes;

    std::unordered_map<vtf_id_t, std::vector<vtf_id_t>> graphCopy = m_graph;
    std::unordered_map<vtf_id_t, vtf_id_t> nodeIndegreeMapCopy = m_nodeIndegreeMap;

    while (true) {
        for (auto &[nodeId, degree] : nodeIndegreeMapCopy) {
            if (degree == 0) {
                sameLevelNodes.push_back(nodeId);
            }
        }

        if (!sameLevelNodes.empty()) {
            for (int nodeId : sameLevelNodes) {
            //erase node info from graph 
                for (vtf_id_t otherNodeId : graphCopy[nodeId]) {
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
                VTF_LOGE("please check node dependcy.");
                //we must clear error node order
                nodeOrder.clear();
            }
            break;
        }
    }
    m_nodeOrderCache = nodeOrder;
    return nodeOrder;
}


std::set<std::vector<vtf_id_t>> DAG::multiTopologicalSort()
{
    rebuildGraphIfNeed();
    std::vector<std::vector<vtf_id_t>> nodeOrder;
    std::vector<vtf_id_t> sameLevelNodes;

    std::unordered_map<vtf_id_t, std::vector<vtf_id_t>> graphCopy = m_graph;
    std::unordered_map<vtf_id_t, vtf_id_t> nodeIndegreeMapCopy = m_nodeIndegreeMap;
    while (true) {
        for (auto &[nodeId, degree] : nodeIndegreeMapCopy) {
            if (degree == 0) {
                sameLevelNodes.push_back(nodeId);
            }
        }

        if (!sameLevelNodes.empty()) {
            for (int nodeId : sameLevelNodes) {
            //erase node info from graph 
                for (vtf_id_t otherNodeId : graphCopy[nodeId]) {
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
    std::set<std::vector<vtf_id_t>> allTopoOrder;
    std::vector<vtf_id_t> topoOrder;
    findAllTopologicalSort(nodeOrder, allTopoOrder, topoOrder, 0);
    return allTopoOrder;
}

void DAG::findAllTopologicalSort(
    std::vector<std::vector<vtf_id_t>>& nodeOrder, 
    std::set<std::vector<vtf_id_t>>& allTopoOrder, 
    std::vector<vtf_id_t>& topoOrder,
    int startIdx
)
{
    if (topoOrder.size() == nodeOrder.size()) {
        std::vector<vtf_id_t> temp;
        for (size_t i = 0; i < topoOrder.size(); i++) {
            if (i > 0 && !checkDependency(topoOrder[i-1], topoOrder[i])) {
                break;
            }
            temp.push_back(topoOrder[i]);
        }
        if (temp.size() > 1) allTopoOrder.insert(temp);
        // allTopoOrder.insert(topoOrder);
        return;
    }

    for (size_t i = startIdx; i < nodeOrder.size(); i++) {
        for (size_t j = 0; j < nodeOrder[i].size(); j++) {
            topoOrder.push_back(nodeOrder[i][j]);
            findAllTopologicalSort(nodeOrder, allTopoOrder, topoOrder, i + 1);
            topoOrder.pop_back();
        } 
    }
}

bool DAG::checkDependency(vtf_id_t srcNodeId, vtf_id_t dstNodeId)
{
    if (m_graph.count(srcNodeId) == 0) {
        return false;
    }
    
    auto connectionsIter = m_graph.find(srcNodeId);
    auto resultIter = std::find(connectionsIter->second.cbegin(), connectionsIter->second.cend(), dstNodeId);
    if (resultIter == connectionsIter->second.cend()) {
        return false;
    }
    return true;
}


void DAG::dumpGraph()
{
    for (auto &[nodeId, outNodeIds] : m_graph) {
        std::stringstream ss;
        ss << nodeId << ": ";
        for (vtf_id_t outNodeId : outNodeIds) {
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

void DAG::clear()
{
    m_graph.clear();
    m_graphModified = true;
    m_nodeIdMap.clear();
    m_nodeIndegreeMap.clear();
    m_nodeOrderCache.clear();
}

}