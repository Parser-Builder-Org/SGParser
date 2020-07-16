# Filename:  CMakeLists.txt
# Content:   CMake file for Calc sample
# Provided AS IS under MIT License; see LICENSE file in root folder.

cmake_minimum_required(VERSION 3.13)

project(calc VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${samples_BINARY_DIR}/Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${samples_BINARY_DIR}/Release")

include_directories(
    .
    "../generated/"
)

add_executable(calc Calc.cpp)

target_link_libraries(calc parser)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(SG_RELEASE_BUILD)
        target_link_options(${PROJECT_NAME} PUBLIC /LTCG /OPT:REF /OPT:ICF)
    else()
        target_link_options(${PROJECT_NAME} PUBLIC /INCREMENTAL:NO)
    endif()
endif()
