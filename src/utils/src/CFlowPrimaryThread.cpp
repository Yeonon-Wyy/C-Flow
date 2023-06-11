#include "CFlowPrimaryThread.h"

namespace cflow::utils::thread {
void CFlowPrimaryThread::reset()
{
    m_stop = true;
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void CFlowPrimaryThread::execute()
{
    while (!m_stop)
    {
        processTask();
        m_totalTaskNum--;
    }
    if (m_stop)
    {
        // process
        while (!m_tasks.empty())
        {
            processTask();
            m_totalTaskNum--;
        }
    }
}

void CFlowPrimaryThread::pushTask(std::function<void()>&& func)
{
    m_totalTaskNum++;
    m_tasks.push(std::move(func));
}

void CFlowPrimaryThread::processTask()
{
    auto task = popTask();
    if (task)
    {
        task();
    }
    else
    {
        // task is invalid, thead yield. avoid cpu busy-loop
        std::this_thread::yield();
    }
}

std::function<void()> CFlowPrimaryThread::popTask()
{
    std::function<void()> task;
    m_tasks.pop(task);
    return task;
}

CFlowPrimaryThread::~CFlowPrimaryThread() { reset(); }
} // namespace cflow::utils::thread