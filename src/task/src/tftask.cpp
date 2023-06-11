#include "tftask.h"
#include "../../utils/include/log.h"
#include "../../utils/include/str_convertor.h"

namespace cflow::task {

TFTask::TFTask(std::shared_ptr<BufferManagerFactory<void>> bufferMgrFactory)
    : Task(),
      DAGNode(ID()),
      m_priority(TaskPriority::NORMAL),
      m_taskType(TaskType::NORMAL),
      m_buffeManagerFactory(bufferMgrFactory)
{
    m_name = TASK_NAME_PREFIX + utils::StringConvetor::digit2String(ID());
}

TFTask::TFTask(TaskCreateInfo&&                            createInfo,
               std::shared_ptr<BufferManagerFactory<void>> bufferMgrFactory)
    : Task(),
      DAGNode(ID()),
      m_processFunc(createInfo.processFunc),
      m_priority(createInfo.priority),
      m_taskType(createInfo.taskType),
      m_buffeManagerFactory(bufferMgrFactory)
{
    if (createInfo.name == "")
    {
        m_name = TASK_NAME_PREFIX + utils::StringConvetor::digit2String(ID());
    }
    else
    {
        m_name = std::move(createInfo.name);
    }
}

void TFTask::operator()()
{
    if (m_processFunc)
    {
        m_processFunc();
    }
    else
    {
        throw std::runtime_error("before execute, must set runable");
    }
}

TFTaskBufferInfoSP TFTask::getBufferInfoByBFS(const BufferSpecification& bfs)
{
    auto bufferMgr = m_buffeManagerFactory->getBufferManager(bfs);
    if (!bufferMgr)
    {
        bufferMgr = m_buffeManagerFactory->createBufferManager(bfs);
    }
    auto bufInfo = bufferMgr->popBuffer();
    return bufInfo;
}

void TFTask::addInputForNode(const BufferSpecification& bfs)
{
    m_nodeBufferInfo.input.push_back(getBufferInfoByBFS(bfs));
}

void TFTask::addOutputForNode(const BufferSpecification& bfs)
{
    m_nodeBufferInfo.input.push_back(getBufferInfoByBFS(bfs));
}
} // namespace cflow::task