cmake_minimum_required(VERSION 3.0)
project(client)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

set(TARGET_NAME "${PROJECT_NAME}")

find_package(Boost 1.60 REQUIRED)

file(GLOB SRCS "*.cpp")

add_executable(${TARGET_NAME} ${SRCS})
target_include_directories(${TARGET_NAME} PUBLIC "include")
target_link_libraries(${TARGET_NAME} PUBLIC)
