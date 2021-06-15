# Filename:  GNU.cmake
# Content:   CMake options for GNU compiler.
# Provided AS IS under MIT License; see LICENSE file in root folder.

# Set minimum required CMake version.
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# Set common options for all configurations.
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# Temporary: suppress `invalid-offsetof` warning since it appears to be a false-positive.
add_compile_options(-Wno-invalid-offsetof)

# Set different compiler options for Debug and Release builds.
if(SG_RELEASE_BUILD)
    add_compile_options(-O3)
else()
    add_compile_options(-g -O0)
endif()
