cmake_minimum_required(VERSION 2.6)
project(vtf)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -DVTF_DEBUG_MODE")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -DNDEBUG")

include_directories("../src/core/")
include_directories("../src/core/pipeline")
include_directories("/usr/include/")


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)


SET(EXECUTABLE_OUTPUT_PATH "../bin/test/")

add_executable(testBlockingQueue "../src/test/blockingQueueTest.cpp")
target_link_libraries(testBlockingQueue PUBLIC Threads::Threads)

add_executable(testDag "../src/test/dagTest.cpp")
target_link_libraries(testDag PUBLIC Threads::Threads)

add_executable(testTaskFlowCtl "../src/test/taskFlowCtlTest.cpp")
target_link_libraries(testTaskFlowCtl PUBLIC Threads::Threads)

add_executable(testTask "../src/test/taskTest.cpp")
target_link_libraries(testTask PUBLIC Threads::Threads)

add_executable(testTaskThreadPool "../src/test/taskThreadPoolTest.cpp")
target_link_libraries(testTaskThreadPool PUBLIC Threads::Threads)

add_executable(testThreadPool "../src/test/threadPoolTest.cpp")
target_link_libraries(testThreadPool PUBLIC Threads::Threads)

add_executable(testThreadLoop "../src/test/threadLoopTest.cpp")
target_link_libraries(testThreadLoop PUBLIC Threads::Threads)

add_executable(testPipeline "../src/test/testPipeline.cpp")
target_link_libraries(testPipeline PUBLIC Threads::Threads)

add_executable(testPipelineWithOpenCV "../src/test/testPipelineWithOpenCV.cpp")
target_link_libraries(testPipelineWithOpenCV PUBLIC Threads::Threads ${OpenCV_LIBS})