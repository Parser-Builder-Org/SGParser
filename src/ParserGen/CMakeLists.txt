# Filename:  CMakeLists.txt
# Content:   CMake project file for Parser Generator.
# Provided AS IS under MIT License; see LICENSE file in root folder.

# Set minimum required CMake version.
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# Set the name of the project.
project(ParserGen CXX)

# Convert project name to lower-case. Will be used as a name of the resulted binary.
string(TOLOWER ${PROJECT_NAME} TARGET_NAME)

# Force output location to be consistent for ninja/VS generators
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/")

# Save a list of all header files in the dedicated variable.
set(HEADER_FILES
    "DFAGen.h"
    "FileInputStream.h"
    "FileOutputStream.h"
    "Grammar.h"
    "Lex.h"
    "Lexeme.h"
    "NFA.h"
    "ParseData.h"
    "ParseMessage.h"
    "ParseTableGen.h"
    "Production.h"
    "RegExprProdEnum.h"
    "RegExpTokenizer.h"
    "StdGrammarLexemeEnum.h"
    "StdGrammarProdEnum.h"
)

# Create a named group of files. Only used by IDEs.
source_group("Header files" FILES ${HEADER_FILES})

# Save a list of all source files in the dedicated variable.
set(SOURCE_FILES
    "DFAGen.cpp"
    "Grammar.cpp"
    "Lex.cpp"
    "NFA.cpp"
    "ParseData.cpp"
    "ParseMessage.cpp"
    "ParseTableGen.cpp"
    "Production.cpp"
)

# Create a named group of files. Only used by IDEs.
source_group("Source files" FILES ${SOURCE_FILES})

# Save a list of all files needed for building a project.
set(ALL_FILES
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

# Add a library target.
add_library(${TARGET_NAME} STATIC ${ALL_FILES})

# Parser is required to be built before the Parser Generator.
add_dependencies(${TARGET_NAME}
    parser
)

# Set paths to public header files.
target_include_directories(${TARGET_NAME}
    PUBLIC .
)

# Link Parser library to Parser Generator binary.
target_link_libraries(${TARGET_NAME} PUBLIC
    parser
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
