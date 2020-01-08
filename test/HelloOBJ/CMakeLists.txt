cmake_minimum_required(VERSION 3.9)
project(HelloOBJ)

add_executable(HelloOBJ HelloOBJ.cpp)
target_compile_definitions(HelloOBJ PRIVATE OBJ_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(HelloOBJ RCube)
