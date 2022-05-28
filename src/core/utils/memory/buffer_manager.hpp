#pragma once
#include "../queue/ring_queue.hpp"
#include "../log/log.hpp"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>

namespace vtf {

struct BufferSpecification 
{
    unsigned long sizeOfBytes;
    unsigned int minQueueSize; //mean cache queue size
    unsigned int maxQueueSize;

    std::size_t hash() const
    {
        static std::size_t hashVal = 
            std::hash<unsigned long>{}(sizeOfBytes) ^
            std::hash<unsigned int>{}(minQueueSize) ^
            std::hash<unsigned int>{}(maxQueueSize);
        return hashVal;
    }
};


struct BufferInfo
{
    unsigned long sizeOfBytes;
    bool isTempAlloctBuffer = false;
    void* ptr;
    bool isValid;

    std::size_t owner;
};


class BufferManager
{
public:

    /**
     * @description: constructor
     * @param {BufferSpecification&} bfs
     * @return {*}
     */    
    BufferManager(const BufferSpecification& bfs)
        :m_bfs(bfs),
         m_bufferQueue(bfs.minQueueSize),
         m_tempBufferQueue(bfs.maxQueueSize - bfs.minQueueSize),
         m_alloctBufferCount(0)
    {
        //pre alloc buffer
        for (size_t i = 0; i < bfs.minQueueSize; i++) {
            auto bf = alloctBuffer();
            m_bufferQueue.push(bf);
        }
    }
    
    /**
     * @description: pop buffer from buffer queue, alias "get buffer from buffer manager
     * @return {*}
     */    
    
    std::shared_ptr<BufferInfo> popBuffer();

    /**
     * @description: push buffer to buffer queue, alias "return buffer to buffer manager
     * @param {shared_ptr<BufferInfo>} bf
     * @return {*}
     */    
    void pushBuffer(std::shared_ptr<BufferInfo> bf);


    /**
     * @description: return current buffer count
     * @param {return} buffer count in buffer queue
     * @return {*}
     */
    unsigned int availableCount() { return m_bufferQueue.size(); }

    /**
     * @description: return no alloct count
     * @return {*}
     */    
    unsigned int noAlloctCount() 
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        unsigned int totalCapcity = (unsigned int)m_bufferQueue.capcity() + (unsigned int)m_tempBufferQueue.capcity();
        if (totalCapcity < m_alloctBufferCount) {
            VTF_LOGE("fatal error!!! system panic!!!");
            std::abort();
        }
        return totalCapcity - m_alloctBufferCount;
    }

    /**
     * @description: if ~BufferManager was called. all buffer(owned by this buffer manager) will be free.
     *               So, user will get nullptr when use buffer after ~BufferManager was called.
     *               I think it's ok. because of RAII, buffer manager control buffer's life.
     * @return {*}
     */    
    ~BufferManager() 
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        freeBufferQueue(m_bufferQueue);
        freeBufferQueue(m_tempBufferQueue);
    }

private:
 
    /**
     * @description: alloc memory
     * @param {bool} isTempBuffer
     * @return {*}
     */
    std::shared_ptr<BufferInfo> alloctBuffer(bool isTempBuffer=false);


    /**
     * @description: free buffer
     * @return {*}
     */    
    void freeBuffer(std::shared_ptr<BufferInfo>& bf);


    /**
     * @description: freeBufferQueue template, used to free buffer in buffer queue.
     * @return {*}
     */    
    template<
        template<typename>
        typename Q
    >
    void freeBufferQueue(Q<std::shared_ptr<BufferInfo>>& q)
    {
        //because ring queue, so size = cap + 1
        int sz = q.capcity() + 1;
        for (int i = 0; i < sz; i++) {
            auto bf = q.at(i);
            freeBuffer(bf);
        }
    }


    /**
     * @description: return no alloct count, no lock
     * @return {*}
     */    
    unsigned int noAlloctCountWithoutLock() 
    {
        //FIXME: a little stupid..... to avoid deadlock..
        unsigned int totalCapcity = (unsigned int)m_bufferQueue.capcity() + (unsigned int)m_tempBufferQueue.capcity();
        if (totalCapcity < m_alloctBufferCount) {
            VTF_LOGE("fatal error!!! system panic!!!");
            std::abort();
        }
        return totalCapcity - m_alloctBufferCount;
    }

private:
    const BufferSpecification m_bfs;
    vtf::RingQueue<std::shared_ptr<BufferInfo>> m_bufferQueue;
    vtf::RingQueue<std::shared_ptr<BufferInfo>> m_tempBufferQueue;
    std::atomic<unsigned int> m_alloctBufferCount;
    std::mutex m_mutex;
};


std::shared_ptr<BufferInfo> BufferManager::popBuffer()
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (m_alloctBufferCount >= m_bfs.maxQueueSize) {
        VTF_LOGE("no enough buffer in buffer queue, pop buffer failed");
        return std::make_shared<BufferInfo>(BufferInfo{.isValid=false});
    }
    if (m_bufferQueue.empty()) {
        //no available buffer now, try alloc new buffer
        VTF_LOGW("no available buffer now, try alloc temp buffer");
        if (!noAlloctCountWithoutLock()) {
            VTF_LOGE("no no enough buffer in buffer queue, try alloc temp buffer failded");
            return std::make_shared<BufferInfo>(BufferInfo{.isValid=false});
        }
        //alloc temp buffer
        auto bf = alloctBuffer(true);
        m_tempBufferQueue.push(bf);

         //FIXME: just for keep "unified queue process"
        m_tempBufferQueue.pop();

        return bf;
    }
    //pop from buffer queue
    auto bf = m_bufferQueue.front();
    m_bufferQueue.pop();
    return bf;

}

void BufferManager::pushBuffer(std::shared_ptr<BufferInfo> bf)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if (bf->owner != m_bfs.hash()) {
        VTF_LOGE("this buffer info is not own by this buffer manager. please check it. buffer owner is {0}, this buffer manager is {1}",
            bf->owner,
            m_bfs.hash());
    }
    if (bf->isTempAlloctBuffer) {
        freeBuffer(bf);
        m_tempBufferQueue.push(bf);
    } else {
        m_bufferQueue.push(bf);
    }
}

std::shared_ptr<BufferInfo> BufferManager::alloctBuffer(bool isTempBuffer)
{
    if (m_bufferQueue.full()) return std::make_shared<BufferInfo>(BufferInfo{.isValid=false});
    BufferInfo bf = {
        .sizeOfBytes = m_bfs.sizeOfBytes,
        .isTempAlloctBuffer = isTempBuffer,
        .ptr = malloc(m_bfs.sizeOfBytes),
        .isValid = true,
        .owner = m_bfs.hash(),
    };
    m_alloctBufferCount++;
    VTF_LOGD("buffer was alloct, now alloct buffer count : {0}", m_alloctBufferCount);

    return std::make_shared<BufferInfo>(bf);
}

void BufferManager::freeBuffer(std::shared_ptr<BufferInfo>& bf)
{
    if (bf->ptr != nullptr) {
        free(bf->ptr);
        bf->ptr = nullptr;
        bf->isValid = false;
        m_alloctBufferCount--;
        VTF_LOGD("buffer was free, now alloct buffer count : {0}", m_alloctBufferCount);
    }
}


} //namespace vtf

