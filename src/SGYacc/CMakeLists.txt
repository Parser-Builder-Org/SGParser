# Filename:  CMakeLists.txt
# Content:   CMake project file for SGYacc.
# Provided AS IS under MIT License; see LICENSE file in root folder.

# Set minimum required CMake version.
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# Set the name of the project.
project(SGYacc CXX)

# Convert project name to lower-case. Will be used as a name of the resulted binary.
string(TOLOWER ${PROJECT_NAME} TARGET_NAME)

# Force output location to be consistent for ninja/VS generators
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/")

# Save a list of all header files in the dedicated variable.
set(HEADER_FILES
    "CmdLineGrammar.h"
    "CmdLineParser.h"
    "CmdLineProdEnum.h"
)

# Create a named group of files. Only used by IDEs.
source_group("Header files" FILES ${HEADER_FILES})

# Save a list of all source files in the dedicated variable.
set(SOURCE_FILES
    "CmdLineParser.cpp"
    "MainSGYacc.cpp"
)

# Create a named group of files. Only used by IDEs.
source_group("Source files" FILES ${SOURCE_FILES})

# Save a list of all files needed for building a project.
set(ALL_FILES
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

# Add an executable target.
add_executable(${TARGET_NAME} ${ALL_FILES})

# Parser and Parser Generator are required to be built before the SGYacc.
add_dependencies(${TARGET_NAME}
    parser
    parsergen
)

# Link Parser and Parser Generator libraries to SGYacc binary.
target_link_libraries(${TARGET_NAME} PUBLIC
    parser
    parsergen
)

# Set specific linker options (only for MSVC-based builds).
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Set different options for Release and Debug builds.
    if(SG_RELEASE_BUILD)
        target_link_options(${TARGET_NAME} PUBLIC /WX /LTCG /OPT:REF /OPT:ICF)
    else()
        target_link_options(${TARGET_NAME} PUBLIC /WX /INCREMENTAL:NO)
    endif()
endif()
