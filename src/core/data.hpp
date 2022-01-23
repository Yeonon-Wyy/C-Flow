/*
 * @Descripttion: 
 * @version: 
 * @Author: yeonon
 * @Date: 2022-01-22 21:41:15
 * @LastEditors: yeonon
 * @LastEditTime: 2022-01-23 19:33:34
 */

#pragma once
#include <vector>
#include "./utils/id_generator.hpp"
#include "type.hpp"

namespace vtf {
/**
 * @name: class Data
 * @Descripttion: it is a data object. 
 *                but this class just a pure virtual class. provider some interface, users must implement these interfaces.
 *                
 * @param {*}
 * @return {*}
 */
class Data {
public:
    Data()
        :m_id(m_idGenerator.generate())
    {}

    virtual ~Data() {
    }


    long ID() { return m_id; };

    /**
     * @name: constructDependency
     * @Descripttion: construct dependency for current Data
     * @param {*}
     * @return {*}
     */    
    virtual bool constructDependency(const std::vector<long>&) = 0;

    /**
     * @name: findNextNodes
     * @Descripttion: find next node list, the list size can only include one nodes or multi nodes, it decide by impl class
     * @param {*}
     * @return {*}
     */    
    // virtual std::vector<long> findNextNodes() = 0;

    /**
     * @name: getCurrentNodes
     * @Descripttion: get current processing node list. the list size can only include one nodes or multi nodes, it decide by impl class
     * @param {*}
     * @return {*}
     */    
    virtual long getCurrentNode() = 0;

    /**
     * @name: getNextNode
     * @Descripttion: get next process node list. the list size can only include one nodes or multi nodes, it decide by impl class
     * @param {*}
     * @return {*}
     */    
    virtual long getNextNode() = 0;


    /**
     * @name: checkDependencyIsReady
     * @Descripttion: check current data state, if is ready will return true, or else will return false
     * @param {*}
     * @return {*}
     */    
    virtual bool checkDependencyIsReady() = 0;

    /**
     * @name: markCurrentNodeReady
     * @Descripttion: mark current node is ready. will effect next node dependency setting
     * @param {*}
     * @return {*}
     */    
    virtual void markCurrentNodeReady() = 0;

    /**
     * @name: scenario
     * @Descripttion: get scenario of this data
     * @param {*}
     * @return {*}
     */    
    virtual uint32_t scenario() = 0;

    /**
     * @name: setNotifyStatus
     * @Descripttion: just set notifier status, default is OK
     * @param {NotifyStatus&&} status
     * @return {*}
     */    
    virtual void setNotifyStatus(NotifyStatus&& status) = 0;

    /**
     * @name: getNotifyStatus
     * @Descripttion: return a NotifyStatus
     * @param {*}
     * @return {*}
     */    
    virtual NotifyStatus getNotifyStatus() = 0;

    /**
     * @name: addNotifierForNode
     * @Descripttion: add a notifier for node, when node done, the specified Notifier will be called when node process done
     * @param {long} notifierId
     * @param {long} nodeId
     * @return {*}
     */    
    virtual void addNotifierForNode(long nodeId, long notifierId) = 0;

    /**
     * @name: getNotifiersByNodeId
     * @Descripttion: get notifiers by node id
     * @param {long} nodeId
     * @return {*}
     */    
    virtual std::vector<long> getNotifiersByNodeId(long nodeId) = 0;
private:
    static vtf::utils::IDGenerator m_idGenerator;
    long m_id;
};

vtf::utils::IDGenerator Data::m_idGenerator;
}