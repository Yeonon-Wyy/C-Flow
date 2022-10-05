cmake_minimum_required(VERSION 2.6)
project(cflow_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER             "/usr/bin/clang++")
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -DCFlow_DEBUG_MODE")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -DNDEBUG")

# user-define log file path
add_definitions( -DLOGFILENAME="/tmp/cflow_log/cflow_log.txt")

include_directories("/usr/include/")


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
# find_package(OpenCV REQUIRED)


SET(EXECUTABLE_OUTPUT_PATH "../bin/")

add_executable(testBlockingQueue "../blockingQueueTest.cpp")
target_link_libraries(testBlockingQueue PUBLIC Threads::Threads)

add_executable(testDag "../dagTest.cpp")
target_link_libraries(testDag PUBLIC Threads::Threads)

add_executable(testTaskFlowCtl "../taskFlowCtlTest.cpp")
target_link_libraries(testTaskFlowCtl PUBLIC Threads::Threads)

add_executable(testTask "../taskTest.cpp")
target_link_libraries(testTask PUBLIC Threads::Threads)

add_executable(testTaskThreadPool "../taskThreadPoolTest.cpp")
target_link_libraries(testTaskThreadPool PUBLIC Threads::Threads)

add_executable(testThreadPool "../threadPoolTest.cpp")
target_link_libraries(testThreadPool PUBLIC Threads::Threads)

add_executable(testThreadLoop "../threadLoopTest.cpp")
target_link_libraries(testThreadLoop PUBLIC Threads::Threads)

add_executable(testPipeline "../testPipeline.cpp")
target_link_libraries(testPipeline PUBLIC Threads::Threads)

add_executable(testScheduler "../testScheduler.cpp")
target_link_libraries(testScheduler PUBLIC Threads::Threads)

add_executable(testRingQueue "../unittest/testRingQueue.cpp")
target_link_libraries(testRingQueue PUBLIC Threads::Threads)

add_executable(testBufferManager "../unittest/testBufferManager.cpp")
target_link_libraries(testBufferManager PUBLIC Threads::Threads)

add_executable(testBuffeManagerFlow "../testBuffeManagerFlow.cpp")
target_link_libraries(testBuffeManagerFlow PUBLIC Threads::Threads)

add_executable(testBufferManagerFactory "../unittest/testBufferManagerFactory.cpp")
target_link_libraries(testBufferManagerFactory PUBLIC Threads::Threads)