# Filename:  CMakeLists.txt
# Content:   CMake project file for Parser
# Provided AS IS under MIT License; see LICENSE file in root folder.

project(Parser CXX)

string(TOLOWER ${PROJECT_NAME} TARGET_NAME)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/Release")

set(HEADER_FILES
    "BacktrackingTokenStream.h"
    "DFATokenizer.h"
    "DFA.h"
    "LexemeInfo.h"
    "MappedTable.h"
    "Parser.h"
    "ParseTable.h"
    "ProductionMask.h"
    "Tokenizer.h"
    "TokenizerBase.h"
    "Kernel/SGDebug.h"
    "Kernel/SGStream.h"
    "Kernel/SGString.h"
)

source_group("Header Files" FILES ${HEADER_FILES})

set(SOURCE_FILES
    "DFA.cpp"
    "ParseTable.cpp"
    "TokenizerBase.cpp"
)

source_group("Source Files" FILES ${SOURCE_FILES})

set(ALL_FILES
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

add_library(${TARGET_NAME} STATIC ${ALL_FILES})

target_include_directories(${TARGET_NAME} 
    PUBLIC .
    PUBLIC "Kernel"
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(SG_RELEASE_BUILD)
        target_link_options(${TARGET_NAME} PUBLIC /LTCG /OPT:REF /OPT:ICF)
    else()
        target_link_options(${TARGET_NAME} PUBLIC /INCREMENTAL:NO)
    endif()
endif()
