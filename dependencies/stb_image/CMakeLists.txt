cmake_minimum_required(VERSION 3.9)
project(stb_image)

add_library(stb_image include/stb_image/stb_image.h src/stb_image.cpp src/stb_image_write.cpp)
target_include_directories(stb_image PUBLIC include/)