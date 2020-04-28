cmake_minimum_required(VERSION 3.9)
project(Pointcloud)

add_executable(Pointcloud Pointcloud.cpp)
target_link_libraries(Pointcloud RCube)
