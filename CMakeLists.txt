cmake_minimum_required(VERSION 3.9)
project(RCube)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories("include")

file(GLOB_RECURSE INC_FILES "include/*.h")							
file(GLOB_RECURSE SRC_FILES "src/*.cpp")

set(RC_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
source_group(TREE ${RC_ROOT} FILES ${INC_FILES})
source_group(TREE ${RC_ROOT} FILES ${SRC_FILES})

add_library(RCube STATIC ${INC_FILES} ${SRC_FILES})
set_property(TARGET RCube PROPERTY CXX_STANDARD 17)
set_property(TARGET RCube PROPERTY CXX_STANDARD_REQUIRED TRUE)
add_definitions(-DNOMINMAX)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

SET(GLM_TEST_ENABLE OFF CACHE BOOL "")
SET(GLM_FORCE_SILENT_WARNINGS ON CACHE BOOL "")
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

option(RCUBE_BUILD_EXAMPLES "Whether to build examples (default: OFF)" ON)
if(RCUBE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
