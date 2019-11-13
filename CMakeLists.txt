cmake_minimum_required(VERSION 3.9)
project(RCube)

include_directories("include")

file(GLOB_RECURSE INC_FILES "include/RCube/*.h"
							"include/RCube/Components/*.h"
                            "include/RCube/Controller/*.h"
							"include/RCube/Core/Arch/*.h"
							"include/RCube/Core/Graphics/*.h"
							"include/RCube/Core/Graphics/Effects/*.h"							
							"include/RCube/Core/Graphics/Materials/*.h"
							"include/RCube/Core/Graphics/MeshGen/*.h"
							"include/RCube/Core/Graphics/OpenGL/*.h"
							"include/RCube/Core/Graphics/Prefilter/*.h"
							"include/RCube/Core/Graphics/TexGen/*.h"
							"include/RCube/Systems/*.h"
							
							"include/RCubeViewer/*.h")
							
file(GLOB_RECURSE SRC_FILES "src/RCube/*.cpp"
							"src/RCube/Components/*.cpp"
                            "src/RCube/Controller/*.cpp"
							"src/RCube/Core/Arch/*.cpp"
							"src/RCube/Core/Graphics/*.cpp"
							"src/RCube/Core/Graphics/Effects/*.cpp"							
							"src/RCube/Core/Graphics/Materials/*.cpp"
							"src/RCube/Core/Graphics/MeshGen/*.cpp"
							"src/RCube/Core/Graphics/OpenGL/*.cpp"
							"src/RCube/Core/Graphics/Prefilter/*.cpp"
							"src/RCube/Core/Graphics/TexGen/*.cpp"
							"src/RCube/Systems/*.cpp"
							
							"src/RCubeViewer/*.cpp")


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

target_include_directories(RCube PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(RCube PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm)
target_include_directories(RCube PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glfw/include)
target_include_directories(RCube PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/include)

target_link_libraries(RCube glm_static glfw glad stb_image)

add_subdirectory(test)
add_subdirectory(apps)
