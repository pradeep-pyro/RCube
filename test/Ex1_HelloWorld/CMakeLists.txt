cmake_minimum_required(VERSION 3.9)
project(HelloWorld)

add_executable(HelloWorld HelloWorld.cpp)
target_link_libraries(HelloWorld RCube)
