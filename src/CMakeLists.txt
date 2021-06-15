# Filename:  CMakeLists.txt
# Content:   CMake project file for Parser, Parser Generator and SGYacc.
# Provided AS IS under MIT License; see LICENSE file in root folder.

# Set minimum required CMake version.
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# Add Parser project file.
add_subdirectory(Parser)

# Add Parser Generator project file.
add_subdirectory(ParserGen)

# Add SGYacc project file.
add_subdirectory(SGYacc)
