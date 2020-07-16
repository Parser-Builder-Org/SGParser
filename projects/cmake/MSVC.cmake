# Filename:  MSVC.cmake
# Content:   CMake options for MSVC compiler
# Provided AS IS under MIT License; see LICENSE file in root folder.

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

if(SG_RELEASE_BUILD)
    add_compile_options(/O2 /Oi /Ot /Gy /GL /W3 /sdl-)
else()
    add_compile_options(/Od /W4)
endif()
