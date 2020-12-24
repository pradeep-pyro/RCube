cmake_minimum_required(VERSION 3.9)
project(PBR)

add_executable(PBR PBR.cpp)
target_compile_definitions(PBR PRIVATE RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(PBR RCube)
