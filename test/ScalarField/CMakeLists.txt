cmake_minimum_required(VERSION 3.9)
project(ScalarField)

add_executable(ScalarField ScalarField.cpp)
target_link_libraries(ScalarField RCube)
