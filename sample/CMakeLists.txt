cmake_minimum_required(VERSION 2.6)
project(cflow_sample)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -DCFlow_DEBUG_MODE")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -DNDEBUG")

add_definitions( -DLOGFILENAME="/tmp/cflow_log/cflow_log.txt")


include_directories("/usr/include/")
include_directories(../../src/)



set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)


SET(EXECUTABLE_OUTPUT_PATH "../bin/")

add_executable(preview ../opencv/preview.cpp)
target_link_libraries(preview PUBLIC Threads::Threads ${OpenCV_LIBS})