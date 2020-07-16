# Filename:  CMakeLists.txt
# Content:   CMake project file for ParserGen
# Provided AS IS under MIT License; see LICENSE file in root folder.

project(ParserGen CXX)

string(TOLOWER ${PROJECT_NAME} TARGET_NAME)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/Release")

set(HEADER_FILES
    "DFAGen.h"
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

source_group("Header Files" FILES ${HEADER_FILES})

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

source_group("Source Files" FILES ${SOURCE_FILES})

set(ALL_FILES
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

add_library(${TARGET_NAME} STATIC ${ALL_FILES})

add_dependencies(${TARGET_NAME}
    parser
)

target_include_directories(${TARGET_NAME} 
    PUBLIC .
)

target_link_libraries(${TARGET_NAME} PUBLIC
    parser
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(SG_RELEASE_BUILD)
        target_link_options(${TARGET_NAME} PUBLIC /LTCG /OPT:REF /OPT:ICF)
    else()
        target_link_options(${TARGET_NAME} PUBLIC /INCREMENTAL:NO)
    endif()
endif()
