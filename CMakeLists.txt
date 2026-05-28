cmake_minimum_required(VERSION 3.15)
project(filesys C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/file
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/util
)

set(CC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ballfre.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/close.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/creat.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/delete.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dir.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FORMAT.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/halt.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/iallfre.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/igetput.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/install.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/log.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/name.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/open.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rdwt.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shell/shell.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file/fsaddr.cc
)

add_executable(shell.exe ${CC_FILES})
