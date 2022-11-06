/*
 * @Descripttion:
 * @version:
 * @Author: yeonon
 * @Date: 2022-01-22 21:41:15
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-11-06 20:08:21
 */

#pragma once
#include <vector>

#include "./utils/id_generator.hpp"
#include "./utils/memory/buffer_manager_factory.hpp"
#include "type.hpp"

namespace cflow {
/**
 * @name: class Task
 * @Descripttion: it is a task object.
 *                but this class just a pure virtual class. provider some
 * interface, users must implement these interfaces.
 *
 * @param {*}
 * @return {*}
 */
class Task
{
public:
    Task() : m_id(m_idGenerator.generate()) {}

    virtual ~Task() {}

    cflow_id_t ID() { return m_id; };

    /**
     * @name: constructDependency
     * @Descripttion: construct dependency for current Task
     * @param {*}
     * @return {*}
     */
    virtual bool constructDependency(
        std::vector<cflow_id_t>&,
        std::shared_ptr<utils::memory::BufferManagerFactory<void>>) = 0;

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
    virtual std::vector<
        std::shared_ptr<utils::memory::BufferManager<void>::BufferInfo>>
    input() = 0;

    /**
     * @name: getOuput
     * @description: get output buffer by current node
     * @return {*} output buffer list
     */
    virtual std::vector<
        std::shared_ptr<utils::memory::BufferManager<void>::BufferInfo>>
    output() = 0;

    /**
     * @name: getCurrentNodes
     * @Descripttion: get current processing node list. the list size can only
     * include one nodes or multi nodes, it decide by impl class
     * @param {*}
     * @return {*}
     */
    virtual cflow_id_t getCurrentNode() = 0;

    /**
     * @name: getNextNode
     * @Descripttion: get next process node list. the list size can only include
     * one nodes or multi nodes, it decide by impl class
     * @param {*}
     * @return {*}
     */
    virtual cflow_id_t getNextNode() = 0;

    /**
     * @name: checkDependencyIsReady
     * @Descripttion: check current task state, if is ready will return true, or
     * else will return false
     * @param {*}
     * @return {*}
     */
    virtual bool checkDependencyIsReady() = 0;

    /**
     * @name: markCurrentNodeReady
     * @Descripttion: mark current node is ready. will effect next node
     * dependency setting
     * @param {*}
     * @return {*}
     */
    virtual void markCurrentNodeReady() = 0;

    /**
     * @name: markError
     * @description: mark current task is error
     * @return {*}
     */
    virtual void markError() = 0;

    /**
     * @name: scenario
     * @Descripttion: get scenario of this task
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
     * @name: setTaskType
     * @Descripttion: set Task type for Task object
     * @param {TaskType&&} reference enum taskType
     * @return {*}
     */
    virtual void setTaskType(TaskType&& taskType) = 0;

    /**
     * @name: getTaskType
     * @Descripttion: get Task type of Task object
     * @param
     * @return {*} reference enum taskType
     */
    virtual TaskType getTaskType() = 0;

    /**
     * @name: setPriority
     * @Descripttion: set priority for task
     * @param {TaskPriority&&} priority
     * @return {*}
     */
    virtual void setPriority(TaskPriority&& priority) = 0;

    /**
     * @name: getPriority
     * @Descripttion: get priority for task
     * @param {*}
     * @return {*} priority
     */
    virtual TaskPriority getPriority() = 0;

    /**
     * @name: addNotifierForNode
     * @Descripttion: add a notifier for node, when node done, the specified
     * Notifier will be called when node process done
     * @param {cflow_id_t} notifierId
     * @param {cflow_id_t} nodeId
     * @return {*}
     */
    virtual void addNotifierForNode(cflow_id_t notifierId, cflow_id_t nodeId = -1) = 0;

    /**
     * @name: getNotifiersByNodeId
     * @Descripttion: get notifiers by node id
     * @param {cflow_id_t} nodeId
     * @return {*}
     */
    virtual std::vector<cflow_id_t> getNotifiersByNodeId(cflow_id_t nodeId) = 0;

    /**
     * @name: getStatus
     * @description: get task status, OK or ERROR, etc...
     * @return {*}
     */
    virtual TaskStatus getStatus() = 0;

private:
    static cflow::utils::IDGenerator m_idGenerator;
    cflow_id_t                       m_id;
};

cflow::utils::IDGenerator Task::m_idGenerator;
} // namespace cflow