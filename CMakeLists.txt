cmake_minimum_required(VERSION 3.16)
project(lain)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(lain src/main.cpp src/image.cpp src/quantizer.cpp src/theme.cpp)