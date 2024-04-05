cmake_minimum_required(VERSION 2.8...3.25)
project(myWebserver)

# 编译成so动态库， 设置动态库路径 放在根目录lib下
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
# 设置调试信息 及11标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

# 定义参与编译的源文件
aux_source_directory(. SRC_LIST)
# 编译动态库
add_library(myWebserver SHARED ${SRC_LIST})

add_executable(webserver ${SRC_LIST})