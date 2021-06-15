# Filename:  CMakeLists.txt
# Content:   CMake file for Calc sample.
# Provided AS IS under MIT License; see LICENSE file in root folder.

# Set minimum required CMake version.
cmake_minimum_required(VERSION 3.13)

# Set project's name, version and programming language.
project(calc VERSION 0.1.0 LANGUAGES CXX)

# Force output location to be consistent for ninja/VS generators
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/")

# Add generated parser files to project.
include_directories(
    .
    "../generated/"
)

# Add an executable target.
add_executable(calc Calc.cpp)

# Link Parser library to Calc binary.
target_link_libraries(calc parser)

# Set specific linker options (only for MSVC-based builds).
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Set different options for Release and Debug builds.
    if(SG_RELEASE_BUILD)
        target_link_options(${PROJECT_NAME} PUBLIC /WX /LTCG /OPT:REF /OPT:ICF)
    else()
        target_link_options(${PROJECT_NAME} PUBLIC /WX /INCREMENTAL:NO)
    endif()
endif()
