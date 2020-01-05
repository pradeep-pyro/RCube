cmake_minimum_required(VERSION 3.9)
project(imgui)

set(imgui_SOURCES imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_widgets.cpp imgui/examples/imgui_impl_glfw.cpp imgui/examples/imgui_impl_opengl3.cpp)

add_compile_definitions(IMGUI_IMPL_OPENGL_LOADER_GLAD)

add_library(imgui STATIC ${imgui_SOURCES})
target_link_libraries(imgui glad glfw)
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/")
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/examples/")