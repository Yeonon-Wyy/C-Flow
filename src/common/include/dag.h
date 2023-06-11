/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2021-09-25 20:35:55
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-11-06 20:59:03
 */
#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "type.h"
#include <cflow/utils/log.h>

namespace cflow {
class DAGNode
{
public:
    /**
     * @name: DAGNode
     * @Descripttion: constructor of DAGNode
     * @param {int} id is node id
     * @return {*}
     */
    DAGNode(cflow_id_t id);

    virtual ~DAGNode() {}

    /**
     * @name: connect
     * @Descripttion: connect set dependency of node, nodeA->connect(NodeB) mean
     * A -> B
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
    cflow_id_t getNodeId() { return m_nodeId; }

    /**
     * @name: getOutNodes
     * @Descripttion: just return outNodes, in c++ 17, it can use copy-elison
     * @param {*}
     * @return {*}
     */
    std::vector<cflow_id_t> getOutNodes() { return m_outNodes; }

private:
    cflow_id_t              m_nodeId;   // node-id
    std::vector<cflow_id_t> m_outNodes; // out-degree
};

class DAG
{
public:
    DAG() : m_graphModified(false) {}

    ~DAG() { CFLOW_LOGD("dag destory"); }

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
    std::vector<std::vector<cflow_id_t>> topologicalSort();

    /**
     * @name: multiTopologicalSort
     * @Descripttion: topological sort algorithem for DAG, will generte multi
     * order
     * @param {*}
     * @return {*} all topological order
     */
    std::set<std::vector<cflow_id_t>> multiTopologicalSort();

    /**
     * @name: graph
     * @Descripttion: return a graph
     * @param {*}
     * @return {*}
     */
    std::unordered_map<cflow_id_t, std::vector<cflow_id_t>> graph()
    {
        return m_graph;
    }
    /**
     * @name: dumpGraph
     * @Descripttion: dump Graph info
     * @param {*}
     * @return {*}
     */
    void dumpGraph();

    /**
     * @name: clear
     * @Descripttion: clear all info of dag, include graph info, node info,
     * etc...
     * @param {*}
     * @return {*}
     */
    void clear();

    /**
     * @name: checkDependency
     * @Descripttion: check Dependency with src node id and dst node id
     * @param {cflow_id_t} srcNodeId
     * @param {cflow_id_t} dstNodeId
     * @return {*}
     */
    bool checkDependency(cflow_id_t srcNodeId, cflow_id_t dstNodeId);

private:
    /**
     * @name: findAllTopologicalSort
     * @Descripttion: find all topological order by node order
     * @param {*}
     * @return {*}
     */
    void findAllTopologicalSort(std::vector<std::vector<cflow_id_t>>& nodeOrder,
                                std::set<std::vector<cflow_id_t>>& allTopoOrder,
                                std::vector<cflow_id_t>&           topoOrder,
                                int                                startIdx);

private:
    std::unordered_map<cflow_id_t, std::vector<cflow_id_t>> m_graph;
    std::unordered_map<cflow_id_t, std::weak_ptr<DAGNode>>  m_nodeIdMap;
    std::unordered_map<cflow_id_t, cflow_id_t>              m_nodeIndegreeMap;
    bool                                                    m_graphModified;
    std::vector<std::vector<cflow_id_t>>                    m_nodeOrderCache;
};

} // namespace cflow