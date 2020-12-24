cmake_minimum_required(VERSION 3.9)
project(Ex2_OBJMesh)

add_executable(Ex2_OBJMesh Ex2_OBJMesh.cpp)
target_compile_definitions(Ex2_OBJMesh PRIVATE OBJ_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(Ex2_OBJMesh RCube)
