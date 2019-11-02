cmake_minimum_required(VERSION 3.9)
project(RCube)

file(GLOB_RECURSE INC_FILES "rcube/*.h")
file(GLOB_RECURSE SRC_FILES "rcube/*.cpp")

add_library(RCube STATIC ${INC_FILES} ${SRC_FILES})

SET(GLM_TEST_ENABLE OFF CACHE BOOL "Disabling GLM tests")
SET(BUILD_SHARED_LIBS OFF CACHE BOOL "Disabling GLM shared library")
add_subdirectory(dependencies/glm EXCLUDE_FROM_ALL)
target_include_directories(RCube PRIVATE dependencies/glm)
target_link_libraries(RCube PRIVATE glm)
add_subdirectory(dependencies/glfw EXCLUDE_FROM_ALL)
target_link_libraries(RCube PRIVATE glfw)
add_subdirectory(dependencies/glad EXCLUDE_FROM_ALL)
target_link_libraries(RCube PRIVATE glad)
add_subdirectory(dependencies/stb_image EXCLUDE_FROM_ALL)
target_link_libraries(RCube PRIVATE stb_image)
