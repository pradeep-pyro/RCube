cmake_minimum_required(VERSION 3.9)
project(OBJMesh)

add_executable(OBJMesh OBJMesh.cpp)
target_compile_definitions(OBJMesh PRIVATE OBJ_RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(OBJMesh RCube)
