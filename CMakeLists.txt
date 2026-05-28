cmake_minimum_required(VERSION 3.15)
project(filesys C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 根目录 .c 文件（主程序 + 工具）
set(C_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/igetput.c
    ${CMAKE_CURRENT_SOURCE_DIR}/iallfre.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ballfre.c
    ${CMAKE_CURRENT_SOURCE_DIR}/name.c
    ${CMAKE_CURRENT_SOURCE_DIR}/access.c
    ${CMAKE_CURRENT_SOURCE_DIR}/log.c
    ${CMAKE_CURRENT_SOURCE_DIR}/close.c
    ${CMAKE_CURRENT_SOURCE_DIR}/creat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/dir.c
    ${CMAKE_CURRENT_SOURCE_DIR}/format.c
    ${CMAKE_CURRENT_SOURCE_DIR}/delete.c
    ${CMAKE_CURRENT_SOURCE_DIR}/open.c
    ${CMAKE_CURRENT_SOURCE_DIR}/halt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/rdwt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/install.c
)

# src/file/ .cc 文件
set(CC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file/cat.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file/dir.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file/file.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file/fsaddr.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file/test.cc
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
add_executable(main.exe ${C_FILES} ${CC_FILES})

# mkfs 工具
add_executable(mkfs.exe ${CMAKE_CURRENT_SOURCE_DIR}/src/file/mkfs.c)
