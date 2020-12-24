cmake_minimum_required(VERSION 3.9)
project(Ex3_IBL)

add_executable(Ex3_IBL Ex3_IBL.cpp)
target_compile_definitions(Ex3_IBL PRIVATE RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(Ex3_IBL RCube)
