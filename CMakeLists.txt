# Filename:  CMakeLists.txt
# Content:   Root CMake project file
# Provided AS IS under MIT License; see LICENSE file in root folder.

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(SGParser CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LOWER)
if(${CMAKE_BUILD_TYPE_LOWER} STREQUAL "release")
    set(SG_RELEASE_BUILD TRUE)
else()
    set(SG_RELEASE_BUILD FALSE)
    add_compile_definitions(SG_BUILD_DEBUG)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    include("projects/cmake/AppleClang.cmake")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    include("projects/cmake/Clang.cmake")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    include("projects/cmake/GNU.cmake")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    include("projects/cmake/MSVC.cmake")
else()
    message(FATAL_ERROR "Unsupported compiler")
endif()

add_subdirectory(src)
add_subdirectory(samples)
