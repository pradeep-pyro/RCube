cmake_minimum_required(VERSION 3.9)
project(imguizmo)

set(imguizmo_SOURCES ImGuizmo/imguizmo.cpp)

add_library(imguizmo STATIC ${imguizmo_SOURCES})
target_compile_features(imguizmo PRIVATE cxx_std_17)
target_link_libraries(imguizmo glad glfw imgui)
target_include_directories(imguizmo PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo/")
