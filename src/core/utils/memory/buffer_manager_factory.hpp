/*
 * @Author: Yeonon
 * @Date: 2022-05-29 15:24:00
 * @LastEditors: Yeonon
 * @LastEditTime: 2022-06-25 18:22:37
 * @FilePath: /src/core/utils/memory/buffer_manager_factory.hpp
 * @Description: 
 * Copyright 2022 Yeonon, All Rights Reserved. 
 * 2022-05-29 15:24:00
 */
#pragma once

#include <memory>
#include <unordered_map>
#include <mutex>

#include "buffer_manager.hpp"



namespace vtf {
namespace utils {
namespace memory {


/**
 * @description: buffer manager factory, provider create,get,release interface for user.
 * @return {*}
 */
template<typename E> 
class BufferManagerFactory
{
public:
    using BufferManagerSp = std::shared_ptr<BufferManager<E>>;


    /**
     * @description: create bufferManaager by BufferSpecification
     * @param {BufferSpecification&} bfs BufferSpecification
     * @return {*}
     */    
    BufferManagerSp createBufferManager(const BufferSpecification& bfs);

    /**
     * @description: get a buffer manager by BufferSpecification, is not exit, just return empty sp, don't create
     * @param {BufferSpecification&} bfs
     * @return {*}
     */    
    BufferManagerSp getBufferManager(const BufferSpecification& bfs);

    /**
     * @description: release a buffer manager
     * @param {BufferSpecification&} bfs
     * @return {*}
     */    
    void releaseBufferManager(const BufferSpecification& bfs);
private:
    std::unordered_map<BufferSpecification, BufferManagerSp, hash_of_bufferSpecification> m_bufferManagerMap;
    std::mutex m_bufferManagerMapLock;
};

template<typename E>
typename BufferManagerFactory<E>::BufferManagerSp BufferManagerFactory<E>::createBufferManager(const BufferSpecification& bfs)
{
    std::unique_lock<std::mutex> lk(m_bufferManagerMapLock);
    if (m_bufferManagerMap.count(bfs)) {
        VTF_LOGD("buffer manager is already exit. don't create");
        return m_bufferManagerMap[bfs];
    }

    auto bmsp = std::make_shared<BufferManager<E>>(bfs);
    m_bufferManagerMap[bfs] = bmsp;
    VTF_LOGD("create a buffer manager {0} success", bfs.name);
    return bmsp;
}

template<typename E>
typename BufferManagerFactory<E>::BufferManagerSp BufferManagerFactory<E>::getBufferManager(const BufferSpecification& bfs)
{
    std::unique_lock<std::mutex> lk(m_bufferManagerMapLock);
    if (m_bufferManagerMap.count(bfs) == 0) {
        VTF_LOGE("buffer manager does not exist. please check buffer specification.");
        return nullptr;
    }
    return m_bufferManagerMap[bfs];
}

template<typename E>
void BufferManagerFactory<E>::releaseBufferManager(const BufferSpecification& bfs)
{
    std::unique_lock<std::mutex> lk(m_bufferManagerMapLock);
    if (m_bufferManagerMap.count(bfs) == 0) {
        VTF_LOGE("buffer manager does not exist. please check buffer specification.");
        return;
    }
    m_bufferManagerMap.erase(bfs);
    VTF_LOGD("release a buffer manager from buffer manager factory, please ensure you don't hold buffer manager.");
}



} //namespace memory
} //namespace utils
} //namespace vtf


