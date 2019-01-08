cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

project(RCube)
file(GLOB_RECURSE INC "*.h")
file(GLOB_RECURSE SRC "*.cpp")

find_package(glfw3 REQUIRED)

add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

add_executable(${PROJECT_NAME} ${INC} ${SRC})
target_link_libraries(${PROJECT_NAME} glad dl glfw ${CMAKE_DL_LIBS})
