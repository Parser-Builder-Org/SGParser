# Filename:  AppleClang.cmake
# Content:   CMake toolchain file for Apple Clang compiler.
# Provided AS IS under MIT License; see LICENSE file in root folder.

# Set minimum required CMake version.
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# Set common options for all configurations.
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# Set different compiler options for Debug and Release builds.
if(SG_RELEASE_BUILD)
    # Sets correct LTO settings specific to compilers.
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    add_compile_options(-O3)
else()
    add_compile_options(-g -O0)
endif()
