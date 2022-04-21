cmake_minimum_required(VERSION 3.9)
project(RCube)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories("include")

file(GLOB_RECURSE INC_FILES "include/*.h")
file(GLOB_RECURSE SRC_FILES "src/*.cpp")
file(GLOB_RECURSE SHADER_FILES "assets/*.hlsl")

set(RC_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
source_group(TREE ${RC_ROOT} FILES ${INC_FILES})
source_group(TREE ${RC_ROOT} FILES ${SRC_FILES})
source_group(TREE ${RC_ROOT} FILES ${SHADER_FILES})

set_source_files_properties(${SHADER_FILES} PROPERTIES VS_TOOL_OVERRIDE "None")
#add_custom_target(assets SOURCES ${SHADER_FILES})
#set_target_properties(assets PROPERTIES FOLDER RCube)

add_library(RCube STATIC ${INC_FILES} ${SRC_FILES} ${SHADER_FILES})

target_compile_features(RCube PUBLIC cxx_std_17)
add_definitions(-DNOMINMAX)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

SET(GLM_TEST_ENABLE OFF CACHE BOOL "")
SET(GLM_SILENT_WARNINGS ON CACHE BOOL "")
SET(BUILD_SHARED_LIBS OFF CACHE BOOL "")
add_subdirectory(dependencies/glm)
add_subdirectory(dependencies/glfw)
add_subdirectory(dependencies/glad)
add_subdirectory(dependencies/stb_image)
add_subdirectory(dependencies/imgui)
add_subdirectory(dependencies/DiligentCore)
add_subdirectory(dependencies/DiligentTools)

target_compile_options(RCube PRIVATE -DUNICODE -DENGINE_DLL)

target_include_directories(RCube PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(RCube PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm)
target_include_directories(RCube PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glfw/include)
target_include_directories(RCube PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/include)
target_include_directories(RCube PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/imgui/include)
#target_include_directories(RCube PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/DiligentCore")
#target_include_directories(RCube PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/DiligentCore/Common/include")
#target_include_directories(RCube PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/DiligentTools)

target_link_libraries(RCube PUBLIC glm_static glfw glad stb_image imgui Diligent-GraphicsTools Diligent-Common Diligent-GraphicsEngine)
if(D3D11_SUPPORTED)
    target_link_libraries(RCube PRIVATE Diligent-GraphicsEngineD3D11-shared)
endif()
if(D3D12_SUPPORTED)
    target_link_libraries(RCube PRIVATE Diligent-GraphicsEngineD3D12-shared)
endif()
if(GL_SUPPORTED)
    target_link_libraries(RCube PRIVATE Diligent-GraphicsEngineOpenGL-shared)
endif()
if(VULKAN_SUPPORTED)
    target_link_libraries(RCube PRIVATE Diligent-GraphicsEngineVk-shared)
endif()

option(RCUBE_BUILD_EXAMPLES "Whether to build examples (default: ON)" ON)
if(RCUBE_BUILD_EXAMPLES)
add_subdirectory(examples)
endif()

copy_required_dlls(RCube)