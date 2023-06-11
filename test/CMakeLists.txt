cmake_minimum_required(VERSION 3.22)
project(cflow_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER             "/usr/bin/g++")
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -DCFlow_DEBUG_MODE")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -DNDEBUG")

# user-define log file path
add_definitions( -DLOGFILENAME="/tmp/cflow_log/cflow_log.txt")

# include cflow
find_package(cflow CONFIG REQUIRED)
include_directories(${cflow_INCLUDE_DIR})

# include /usr/include
include_directories("/usr/include/")

# include thread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

SET(EXECUTABLE_OUTPUT_PATH "../bin/")

# include cflow
set(CMAKE_PREFIX_PATH /usr/local/lib/cmake/cflow)

message("CMAKE_PREFIX_PATH " ${CMAKE_PREFIX_PATH})
find_package(cflow_utils REQUIRED)
if(cflow_utils_FOUND)
	message(STATUS "cflow_utils_FOUND = ${cflow_utils_FOUND}")
	message(STATUS "cflow_utils_INCLUDE_DIRS = ${cflow_utils_INCLUDE_DIRS}")
	message(STATUS "cflow_utils_LIBRARIES = ${cflow_utils_LIBRARIES}")
	message(STATUS "cflow_utils_LIBRARY_DIRS = ${cflow_utils_LIBRARY_DIRS}")
endif()
find_package(cflow_common REQUIRED)
if(cflow_common_FOUND)
	message(STATUS "cflow_common_FOUND = ${cflow_common_FOUND}")
	message(STATUS "cflow_common_INCLUDE_DIRS = ${cflow_common_INCLUDE_DIRS}")
	message(STATUS "cflow_common_LIBRARIES = ${cflow_common_LIBRARIES}")
	message(STATUS "cflow_common_LIBRARY_DIRS = ${cflow_common_LIBRARY_DIRS}")
endif()
find_package(cflow_task REQUIRED)
if(cflow_task_FOUND)
	message(STATUS "cflow_task_FOUND = ${cflow_task_FOUND}")
	message(STATUS "cflow_task_INCLUDE_DIRS = ${cflow_task_INCLUDE_DIRS}")
	message(STATUS "cflow_task_LIBRARIES = ${cflow_task_LIBRARIES}")
	message(STATUS "cflow_task_LIBRARY_DIRS = ${cflow_task_LIBRARY_DIRS}")
endif()
find_package(cflow_pipeline REQUIRED)
if(cflow_pipeline_FOUND)
	message(STATUS "cflow_pipeline_FOUND = ${cflow_pipeline_FOUND}")
	message(STATUS "cflow_pipeline_INCLUDE_DIRS = ${cflow_pipeline_INCLUDE_DIRS}")
	message(STATUS "cflow_pipeline_LIBRARIES = ${cflow_pipeline_LIBRARIES}")
	message(STATUS "cflow_pipeline_LIBRARY_DIRS = ${cflow_pipeline_LIBRARY_DIRS}")
endif()
link_directories(${cflow_task_LIBRARY_DIRS}/cflow)

# if need tracy profiler
if(DEFINED ENV{NEED_TRACY_PROFILER})
    # include tracy
    add_subdirectory(${cflow_INCLUDE_DIR}/cflow/3rdparty/tracy .)

    # set dependency
    
    set(TARGET_DEPENDCY 
        PUBLIC Threads::Threads
        cflow::cflow
        Tracy::TracyClient
    )
else()
    # set dependency
    set(TARGET_DEPENDCY 
        PUBLIC Threads::Threads
        ${cflow_common_LIBRARIES}
        ${cflow_utils_LIBRARIES}
        ${cflow_task_LIBRARIES}
        ${cflow_pipeline_LIBRARIES}
    )
endif()


# # add executable
add_executable(testBlockingQueue "../blockingQueueTest.cpp")
target_link_libraries(testBlockingQueue ${TARGET_DEPENDCY})

add_executable(testDag "../dagTest.cpp")
target_link_libraries(testDag ${TARGET_DEPENDCY})

add_executable(testTaskFlowCtl "../taskFlowCtlTest.cpp")
target_link_libraries(testTaskFlowCtl ${TARGET_DEPENDCY})

add_executable(testTask "../taskTest.cpp")
target_link_libraries(testTask ${TARGET_DEPENDCY})

# add_executable(testTaskThreadPool "../taskThreadPoolTest.cpp")
# target_link_libraries(testTaskThreadPool PUBLIC Threads::Threads)

add_executable(testThreadPool "../threadPoolTest.cpp")
target_link_libraries(testThreadPool ${TARGET_DEPENDCY})

add_executable(testThreadLoop "../threadLoopTest.cpp")
target_link_libraries(testThreadLoop ${TARGET_DEPENDCY})

add_executable(testPipeline "../testPipeline.cpp")
target_link_libraries(testPipeline ${TARGET_DEPENDCY})

add_executable(testScheduler "../testScheduler.cpp")
target_link_libraries(testScheduler ${TARGET_DEPENDCY})

add_executable(testRingQueue "../unittest/testRingQueue.cpp")
target_link_libraries(testRingQueue ${TARGET_DEPENDCY})

add_executable(testBufferManager "../unittest/testBufferManager.cpp")
target_link_libraries(testBufferManager ${TARGET_DEPENDCY})

add_executable(testBuffeManagerFlow "../testBuffeManagerFlow.cpp")
target_link_libraries(testBuffeManagerFlow ${TARGET_DEPENDCY})

add_executable(testBufferManagerFactory "../unittest/testBufferManagerFactory.cpp")
target_link_libraries(testBufferManagerFactory ${TARGET_DEPENDCY})