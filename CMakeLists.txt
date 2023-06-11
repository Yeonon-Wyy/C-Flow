cmake_minimum_required(VERSION 3.22)
project(cflow-demo)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -DNDEBUG")


# aux_source_directory(. DIR_SRCS)
add_subdirectory(src/common)
add_subdirectory(src/utils)
add_subdirectory(src/task)
add_subdirectory(src/pipeline)


# include_directories(${PROJECT_SOURCE_DIR}/src)


# set(THREADS_PREFER_PTHREAD_FLAG ON)
# find_package(Threads REQUIRED)


# add_executable(${PROJECT_NAME} main.cpp)
# target_link_libraries(
#     ${PROJECT_NAME} 
#     Threads::Threads
#     cflow_common
#     cflow_utils
#     cflow_task
#     cflow_pipeline
# )
