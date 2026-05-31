cmake_minimum_required(VERSION 3.15)
project(filesys CXX)

set(CMAKE_CXX_STANDARD 17)

# 根目录 .cc 文件（主程序 + 工具）
set(C_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/igetput.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/iallfre.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ballfre.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/name.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/log.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/close.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/creat.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dir.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FORMAT.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/delete.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/open.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/halt.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rdwt.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/install.cc
)

# src/file/ .cc 文件
set(CC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file/cat.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file/file.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file/fsaddr.cc
)

# src/util/ .cc 文件
set(UTIL_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util/nano.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util/utf8.cc
)

# 头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/file
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/util
)

# 主程序
add_executable(main ${C_FILES} ${CC_FILES} ${UTIL_FILES})
