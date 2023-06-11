#ifdef USE_THREAD_BUSY
#include "threadPool_busy.h"

namespace cflow::utils::thread {
ThreadPool::ThreadPool(size_t threadSize) : isStop(false), m_curIdx(0)
{
    for (size_t i = 0; i < threadSize; i++)
    {
        m_workers.emplace_back(new CFlowPrimaryThread());
    }
}

ThreadPool::~ThreadPool()
{
    if (isStop)
    {
        return;
    }

    stop();
}

void ThreadPool::stop()
{
    isStop = true;
    // wait all thread run complate
    m_workers.clear();
}

int ThreadPool::dispatch()
{
    auto selectThread = [this]() {
        int selectIdx = m_curIdx++;
        if (m_curIdx >= m_workers.size())
        {
            m_curIdx = 0;
        }
        int curThreadLoad = m_workers[selectIdx]->totalTaskNum();
        for (int i = 0; i < m_workers.size(); i++)
        {
            if (i == selectIdx) continue;
            int otherThreadLoad = m_workers[i]->totalTaskNum();
            if (curThreadLoad - otherThreadLoad > 4)
            {
                return -1;
            }
        }
        return selectIdx;
    };

    int selectThreadIdx = -1;
    do
    {
        selectThreadIdx = selectThread();
    } while (selectThreadIdx < 0);

    return selectThreadIdx;
}
}; // namespace cflow::utils::thread
#endif