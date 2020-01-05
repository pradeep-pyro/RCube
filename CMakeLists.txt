cmake_minimum_required(VERSION 3.9)
project(RCube)

include_directories("include")

file(GLOB_RECURSE INC_FILES "include/*.h")							
file(GLOB_RECURSE SRC_FILES "src/*.cpp")


add_library(RCube STATIC ${INC_FILES} ${SRC_FILES})
set_property(TARGET RCube PROPERTY CXX_STANDARD 14)
set_property(TARGET RCube PROPERTY CXX_STANDARD_REQUIRED TRUE)
add_definitions(-DNOMINMAX)

SET(GLM_TEST_ENABLE OFF CACHE BOOL "")
SET(BUILD_SHARED_LIBS OFF CACHE BOOL "")
add_subdirectory(dependencies/glm)
add_subdirectory(dependencies/glfw)
add_subdirectory(dependencies/glad)
add_subdirectory(dependencies/stb_image)
add_subdirectory(dependencies/imgui)

target_include_directories(RCube PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(RCube PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm)
target_include_directories(RCube PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glfw/include)
target_include_directories(RCube PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/include)
target_include_directories(RCube PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui/include)

target_link_libraries(RCube glm_static glfw glad stb_image imgui)

add_subdirectory(test)
add_subdirectory(apps)
