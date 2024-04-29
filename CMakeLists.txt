cmake_minimum_required(VERSION 2.8...3.25)
project(myWebserver)



# 添加搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/src/net
    ${PROJECT_SOURCE_DIR}/src/http
    ${PROJECT_SOURCE_DIR}/src/logger
    )
# 编译成so动态库， 设置动态库路径 放在根目录lib下
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
# 设置调试信息 及11标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

# 定义参与编译的源文件
# 包含这些目录下的.cc文件
aux_source_directory(${PROJECT_SOURCE_DIR}/src/net SRC_NET)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/http SRC_HTTP)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/logger SRC_LOG)
# 编译动态库
add_library(myWebserver SHARED ${SRC_NET} ${SRC_LOG} ${SRC_HTTP})

# 目标动态库所需连接的库（这里需要连接libpthread.so）
target_link_libraries(myWebserver pthread)

# 设置安装的默认路径
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

# 将目录安装到 include/tiny_network 下（只安装头文件）
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/ DESTINATION include/myWebserver
        FILES_MATCHING PATTERN "*.h")
        
add_executable(webserver ${SRC_NET} ${SRC_LOG} ${SRC_HTTP})