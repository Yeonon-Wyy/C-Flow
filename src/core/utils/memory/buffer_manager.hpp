#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>

#include "../../../core/type.hpp"
#include "../log/log.hpp"
#include "../queue/ring_queue.hpp"

namespace cflow
{
namespace utils
{
namespace memory
{
struct BufferSpecification
{
    const unsigned long sizeOfBytes;
    const unsigned int  minQueueSize;  // cache queue size
    const unsigned int  maxQueueSize;  // max queue size
    const std::string   name;

    /**
     * @description:  calc hash value
     * @return {*}
     */
    std::size_t hash() const
    {
        static std::size_t hashVal = std::hash<unsigned long>{}(sizeOfBytes) ^ std::hash<unsigned int>{}(minQueueSize) ^ std::hash<unsigned int>{}(maxQueueSize) ^ std::hash<std::string>{}(name);
        return hashVal;
    }

    bool operator==(const BufferSpecification& other) const
    {
        return sizeOfBytes == other.sizeOfBytes && minQueueSize == other.minQueueSize && maxQueueSize == other.maxQueueSize && name == other.name;
    }
};

struct hash_of_bufferSpecification
{
    std::size_t operator()(const BufferSpecification& bfs) const { return bfs.hash(); }
};

template <typename E>
class BufferManager
{
public:
    struct BufferInfo
    {
        unsigned long sizeOfBytes;
        bool          isTempAlloctBuffer = false;
        E*            ptr;
        bool          isValid;
        std::size_t   owner;
        std::string   name = "None";
        unsigned int  numOfUser;
    };

public:
    /**
     * @description: constructor
     * @param {BufferSpecification&} bfs
     * @return {*}
     */
    BufferManager(const BufferSpecification& bfs) : m_bfs(bfs), m_bufferQueue(bfs.minQueueSize), m_tempBufferQueue(bfs.maxQueueSize - bfs.minQueueSize), m_alloctBufferCount(0), m_name(bfs.name)
    {
        // pre alloc buffer
        for (size_t i = 0; i < bfs.minQueueSize; i++)
        {
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
     * @param {shared_ptr<BufferInfo<E>>} bf
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
        std::unique_lock<std::mutex> lk(m_bufferLock);
        unsigned int                 totalCapcity = (unsigned int)m_bufferQueue.capcity() + (unsigned int)m_tempBufferQueue.capcity();
        if (totalCapcity < m_alloctBufferCount)
        {
            CFLOW_LOGE("fatal error!!! system panic!!!");
            std::abort();
        }
        return totalCapcity - m_alloctBufferCount;
    }

    /**
     * @description: add user of buffer
     * @param {shared_ptr<BufferInfo>} bf bufferInfo
     * @param {int} numOfUser number of user
     * @return {*}
     */
    void addUserOfBuffer(std::shared_ptr<BufferInfo> bf, int numOfUser)
    {
        std::unique_lock<std::mutex> lk(m_bufferLock);
        bf->numOfUser += numOfUser;
    }

    /**
     * @description: return buffer manager's name
     * @return {*}
     */
    std::string name() const { return m_name; }

    /**
     * @description: if ~BufferManager was called. all buffer(owned by this buffer manager) will be free.
     *               So, user will get nullptr when use buffer after ~BufferManager was called.
     *               I think it's ok. because of RAII, buffer manager control buffer's life.
     * @return {*}
     */
    ~BufferManager()
    {
        std::unique_lock<std::mutex> lk(m_bufferLock);
        // set m_exit flag and notify waiting thread(if have).
        m_exit = true;
        // just notify, don't need wait, because if bufferManager exit. no any methond to process buffer queue, so we can clear buffer queue safely.
        m_bufferReadyCV.notify_all();
        freeBufferQueue(m_bufferQueue);
        freeBufferQueue(m_tempBufferQueue);
    }

private:
    /**
     * @description: alloc memory
     * @param {bool} isTempBuffer
     * @return {*}
     */
    std::shared_ptr<BufferInfo> alloctBuffer(bool isTempBuffer = false);

    /**
     * @description: free buffer
     * @return {*}
     */
    void freeBuffer(std::shared_ptr<BufferInfo>& bf);

    /**
     * @description: freeBufferQueue template, used to free buffer in buffer queue.
     * @return {*}
     */
    template <template <typename> typename Q>
    void freeBufferQueue(Q<std::shared_ptr<BufferInfo>>& q)
    {
        // because ring queue, so size = cap + 1
        int sz = q.capcity() + 1;
        for (int i = 0; i < sz; i++)
        {
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
        // FIXME: a little stupid..... to avoid deadlock..
        unsigned int totalCapcity = (unsigned int)m_bufferQueue.capcity() + (unsigned int)m_tempBufferQueue.capcity();
        if (totalCapcity < m_alloctBufferCount)
        {
            CFLOW_LOGE("fatal error!!! system panic!!!");
            std::abort();
        }
        return totalCapcity - m_alloctBufferCount;
    }

private:
    const BufferSpecification                                 m_bfs;
    cflow::utils::queue::RingQueue<std::shared_ptr<BufferInfo>> m_bufferQueue;

    // Note: Q: why need tempBufferQueue?
    //      A: because I want manage all of the buffer from bufferManager. Such as When bufferManager exit(dctr), the buffer should free whatever the buffer push or not.
    //         So, I need a container to save it.
    cflow::utils::queue::RingQueue<std::shared_ptr<BufferInfo>> m_tempBufferQueue;

    std::atomic<unsigned int> m_alloctBufferCount;
    std::mutex                m_bufferLock;
    std::condition_variable   m_bufferReadyCV;
    bool                      m_exit = false;

    std::string m_name;
};

template <typename E>
std::shared_ptr<typename BufferManager<E>::BufferInfo> BufferManager<E>::popBuffer()
{
    std::unique_lock<std::mutex> lk(m_bufferLock);

    m_bufferReadyCV.wait(lk, [this]() {
        bool hasCacheBuffer   = !m_bufferQueue.empty();
        bool hasNoAllocBuffer = noAlloctCountWithoutLock() > 0;
        CFLOW_LOGD("hasCacheBuffer: {0}, hasNoAllocBuffer: {1}", hasCacheBuffer, hasNoAllocBuffer);
        return hasCacheBuffer || hasNoAllocBuffer || m_exit;
    });

    if (m_exit)
    {
        CFLOW_LOGD("buffer manager exit.. will free all buffer soon");
        return std::make_shared<BufferInfo>(BufferInfo{.isValid = false});
    }

    if (m_bufferQueue.empty())
    {
        // no available buffer now, alloc new temp buffer
        CFLOW_LOGE("no available cache buffer now, alloc temp buffer");
        // alloc temp buffer
        auto bf = alloctBuffer(true);
        m_tempBufferQueue.push(bf);

        // FIXME: just for keep "unified queue process"
        m_tempBufferQueue.pop();
        bf->numOfUser++;
        return bf;
    }
    // pop from buffer queue
    auto bf = m_bufferQueue.front();
    bf->numOfUser++;
    m_bufferQueue.pop();
    return bf;
}

template <typename E>
void BufferManager<E>::pushBuffer(std::shared_ptr<BufferManager<E>::BufferInfo> bf)
{
    std::unique_lock<std::mutex> lk(m_bufferLock);
    if (bf->owner != m_bfs.hash())
    {
        CFLOW_LOGE("this buffer info is not own by this buffer manager. please check it. buffer owner is {0}, this buffer manager is {1}", bf->owner, m_bfs.hash());
    }
    if (--bf->numOfUser == 0)
    {
        if (bf->isTempAlloctBuffer)
        {
            freeBuffer(bf);
            m_tempBufferQueue.push(bf);
            // FIXME: just for keep "unified queue process"
            m_tempBufferQueue.pop();
        }
        else
        {
            m_bufferQueue.push(bf);
        }
        m_bufferReadyCV.notify_all();
    }
}

template <typename E>
std::shared_ptr<typename BufferManager<E>::BufferInfo> BufferManager<E>::alloctBuffer(bool isTempBuffer)
{
    if (m_bufferQueue.full()) return std::make_shared<BufferInfo>(BufferInfo{.isValid = false});
    BufferInfo bf = {
        .sizeOfBytes        = m_bfs.sizeOfBytes,
        .isTempAlloctBuffer = isTempBuffer,
        .isValid            = true,
        .owner              = m_bfs.hash(),
        .name               = m_bfs.name,
        .numOfUser          = 0,
    };

    // if constexpr statment can instead of std::enable_if
    if constexpr (std::is_same_v<void, E>)
    {
        bf.ptr = malloc(m_bfs.sizeOfBytes);
    }
    else if constexpr (is_default_constructible<E>::value)
    {
        // if user provide a default constructor
        bf.ptr = new E();
    }
    else
    {
        // or else, just use malloc to alloct memory
        bf.ptr = (E*)malloc(sizeof(E));
    }

    m_alloctBufferCount++;
    CFLOW_LOGD("buffer was alloct, now alloct buffer count : {0}", m_alloctBufferCount);

    return std::make_shared<BufferInfo>(bf);
}

template <typename E>
void BufferManager<E>::freeBuffer(std::shared_ptr<BufferInfo>& bf)
{
    if (!bf) return;
    if (bf->ptr != nullptr)
    {
        if constexpr (std::is_same_v<void, E>)
        {
            free(bf->ptr);
        }
        else if constexpr (is_default_constructible<E>::value)
        {
            delete bf->ptr;
        }
        else
        {
            free(bf->ptr);
        }
        bf->ptr     = nullptr;
        bf->isValid = false;
        m_alloctBufferCount--;
        CFLOW_LOGD("buffer was free, now alloct buffer count : {0}", m_alloctBufferCount);
    }
}

}  // namespace memory
}  // namespace utils
}  // namespace cflow
