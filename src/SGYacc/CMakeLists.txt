# Filename:  CMakeLists.txt
# Content:   CMake project file for SGYacc
# Provided AS IS under MIT License; see LICENSE file in root folder.

project(SGYacc CXX)

string(TOLOWER ${PROJECT_NAME} TARGET_NAME)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/Release")

set(HEADER_FILES
    "CmdLineGrammar.h"
    "CmdLineParser.h"
    "CmdLineProdEnum.h"
    "FileStream.h"
)

source_group("Header Files" FILES ${HEADER_FILES})

set(SOURCE_FILES
    "CmdLineParser.cpp"
    "MainSGYacc.cpp"
)

source_group("Source Files" FILES ${SOURCE_FILES})

set(ALL_FILES
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

add_executable(${TARGET_NAME} ${ALL_FILES})

add_dependencies(${TARGET_NAME}
    parser
    parsergen
)

target_link_libraries(${TARGET_NAME} PUBLIC
    parser
    parsergen
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(SG_RELEASE_BUILD)
        target_link_options(${TARGET_NAME} PUBLIC /LTCG /OPT:REF /OPT:ICF)
    else()
        target_link_options(${TARGET_NAME} PUBLIC /INCREMENTAL:NO)
    endif()
endif()
