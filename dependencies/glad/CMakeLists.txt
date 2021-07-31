cmake_minimum_required(VERSION 3.9)
project(glad)

add_library(glad include/glad/glad.h include/KHR/khrplatform.h src/glad.c)
target_compile_features(glad PRIVATE cxx_std_17)
target_include_directories(glad PUBLIC include/)
