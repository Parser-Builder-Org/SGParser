# Filename:  GNU.cmake
# Content:   CMake options for GNU compiler
# Provided AS IS under MIT License; see LICENSE file in root folder.

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if(SG_RELEASE_BUILD)
    add_compile_options(-O3 -Wall -flto)
else()
    add_compile_options(-g -O0 -Wall -Wextra -Wpedantic)
endif()
