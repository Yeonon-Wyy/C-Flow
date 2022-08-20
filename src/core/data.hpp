/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2022-01-22 21:41:15
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-07-03 15:01:20
 */

#pragma once
#include <vector>

#include "./utils/id_generator.hpp"
#include "./utils/memory/buffer_manager_factory.hpp"
#include "type.hpp"

namespace vtf
{
/**
 * @name: class Data
 * @Descripttion: it is a data object.
 *                but this class just a pure virtual class. provider some interface, users must implement these interfaces.
 *
 * @param {*}
 * @return {*}
 */
class Data
{
public:
    Data() : m_id(m_idGenerator.generate()) {}

    virtual ~Data() {}

    vtf_id_t ID() { return m_id; };

    /**
     * @name: constructDependency
     * @Descripttion: construct dependency for current Data
     * @param {*}
     * @return {*}
     */
    virtual bool constructDependency(const std::vector<vtf_id_t>&, std::shared_ptr<utils::memory::BufferManagerFactory<void>>) = 0;

    /**
     * @name: constructIO
     * @description: construct input and output for nodes in pipeline
     * @return {*}
     */
    virtual bool constructIO() = 0;

    /**
     * @name: getInput
     * @description: get input buffer by current node
     * @return {*} input buffer list
     */
    virtual std::vector<std::shared_ptr<utils::memory::BufferManager<void>::BufferInfo>> input() = 0;

    /**
     * @name: getOuput
     * @description: get output buffer by current node
     * @return {*} output buffer list
     */
    virtual std::vector<std::shared_ptr<utils::memory::BufferManager<void>::BufferInfo>> output() = 0;

    /**
     * @name: getCurrentNodes
     * @Descripttion: get current processing node list. the list size can only include one nodes or multi nodes, it decide by impl class
     * @param {*}
     * @return {*}
     */
    virtual vtf_id_t getCurrentNode() = 0;

    /**
     * @name: getNextNode
     * @Descripttion: get next process node list. the list size can only include one nodes or multi nodes, it decide by impl class
     * @param {*}
     * @return {*}
     */
    virtual vtf_id_t getNextNode() = 0;

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
     * @name: setDataType
     * @Descripttion: set Data type for Data object
     * @param {DataType&&} reference enum dataType
     * @return {*}
     */
    virtual void setDataType(DataType&& dataType) = 0;

    /**
     * @name: getDataType
     * @Descripttion: get Data type of Data object
     * @param
     * @return {*} reference enum dataType
     */
    virtual DataType getDataType() = 0;

    /**
     * @name: setPriority
     * @Descripttion: set priority for data
     * @param {DataPriority&&} priority
     * @return {*}
     */
    virtual void setPriority(DataPriority&& priority) = 0;

    /**
     * @name: getPriority
     * @Descripttion: get priority for data
     * @param {*}
     * @return {*} priority
     */
    virtual DataPriority getPriority() = 0;

    /**
     * @name: addNotifierForNode
     * @Descripttion: add a notifier for node, when node done, the specified Notifier will be called when node process done
     * @param {vtf_id_t} notifierId
     * @param {vtf_id_t} nodeId
     * @return {*}
     */
    virtual void addNotifierForNode(vtf_id_t nodeId, vtf_id_t notifierId) = 0;

    /**
     * @name: getNotifiersByNodeId
     * @Descripttion: get notifiers by node id
     * @param {vtf_id_t} nodeId
     * @return {*}
     */
    virtual std::vector<vtf_id_t> getNotifiersByNodeId(vtf_id_t nodeId) = 0;

private:
    static vtf::utils::IDGenerator m_idGenerator;
    vtf_id_t                       m_id;
};

vtf::utils::IDGenerator Data::m_idGenerator;
}  // namespace vtf