cmake_minimum_required(VERSION 3.22)
project(cflow_utils)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -DNDEBUG")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../lib/)


# ADD_DEFINITIONS(-DUSE_THREAD_BUSY)

include_directories(
    ${PROJECT_SOURCE_DIR}/include/ 
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_compile_options(-fPIC)
aux_source_directory(src DIR_LIB_SRCS)
add_library (${PROJECT_NAME} SHARED  ${DIR_LIB_SRCS})

target_include_directories(${PROJECT_NAME} 
     PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> 
     $<INSTALL_INTERFACE:include>)

target_link_libraries(${PROJECT_NAME} Threads::Threads)

FILE(GLOB ALL_HEADER ${PROJECT_SOURCE_DIR}/include/*.h)

install(TARGETS ${PROJECT_NAME} DESTINATION lib/cflow/)
install(FILES ${ALL_HEADER} DESTINATION include/cflow/utils/)


#======================生成 xxxConfig.cmake===============================
# 该变量会通过xxxConfig.cmake.in用于在生成的xxxConfig.cmake中
set(INCLUDE_DIRS include)
set(LIBRARIES ${PROJECT_NAME})
set(LIB_DIR lib)

# 由cmake提供
include(CMakePackageConfigHelpers)

# 生成 xxxConfigVersion.cmake文件
write_basic_package_version_file(
	${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
	VERSION 1.0.0
	COMPATIBILITY SameMajorVersion
)

message(${PROJECT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in)
message(${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake)


# 用于生成 xxxConfig.cmake文件
configure_package_config_file(
	${PROJECT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
	${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
	INSTALL_DESTINATION lib/cmake/cflow
	PATH_VARS INCLUDE_DIRS LIBRARIES LIB_DIR
	INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
)

install(
	FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
	DESTINATION lib/cmake/cflow
)