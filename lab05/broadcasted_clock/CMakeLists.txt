cmake_minimum_required(VERSION 3.5)
project(broadcasted_clock)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(client client.cpp)
add_executable(server server.cpp)
