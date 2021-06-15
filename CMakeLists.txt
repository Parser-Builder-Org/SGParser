# Filename:  CMakeLists.txt
# Content:   Root CMake project file.
# Provided AS IS under MIT License; see LICENSE file in root folder.

# Set minimum required CMake version.
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# Set the name of the project.
project(SGParser CXX)

# Enable grouping of targets in an IDEs, e.g. Visual Studio, CLion, etc.
# Do not have an effect on compiler and linker.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Strict C++17 is required.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# No vendor-dependent extensions are allowed.
set(CMAKE_CXX_EXTENSIONS OFF)

# Set Debug build by default.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set SG_RELEASE_BUILD for the conditional setting of compiler and linker options.
string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LOWER)
if(${CMAKE_BUILD_TYPE_LOWER} MATCHES "(release|relwithdebinfo|minsizerel)")
    set(SG_RELEASE_BUILD TRUE)
else()
    set(SG_RELEASE_BUILD FALSE)
    # This definition is required for proper Debug/Release functionality.
    add_compile_definitions(SG_BUILD_DEBUG)
endif()

# Include compiler-dependent options from the dedicated toolchain files.
if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    include("projects/cmake/AppleClang.cmake")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    include("projects/cmake/Clang.cmake")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    include("projects/cmake/GNU.cmake")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    include("projects/cmake/MSVC.cmake")
else()
    # Only Apple Clang, Clang, GCC and MSVC compilers are supported.
    message(FATAL_ERROR "Unsupported compiler")
endif()

# Add Parser, Parser Generator and SGYacc project files.
add_subdirectory(src)

# Add Samples project files.
add_subdirectory(samples)
