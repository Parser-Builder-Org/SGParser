<!-- 
Filename:  CONTRIBUTING.md
Content:   Information about how to contribute to SGParser project
Provided AS IS under MIT License; see LICENSE file in root folder.
-->
# Contribution Guidelines

You would like to see a feature implemented or a bug fixed in SGParser?
We'd love to accept your patches!
Contributions to SGParser are highly appreciated, be it in the form of general ideas, concrete suggestions, or code patches.

This document explains how you can contribute. By carefully reading it, you minimize the time it takes on your or our side until a modification is implemented. The following guidelines clarify what it takes for a contribution to be accepted.

## General Considerations

In case of any questions or doubts (and after reading all the available resources), feel free to ask. Before opening GitHub issues and/or pull requests, discuss your matter.

If a feature can easily be added on top of SGParser without modifying it, it is probably not a good candidate to be integrated.

## Contributing A Patch

1. Submit an issue describing your proposed change to the issue tracker.
1. Please don't mix more than one logical change per submittal, because it makes the history hard to follow. If you want to make a change that doesn't have a corresponding issue in the issue tracker, please create one.
1. Also, coordinate with team members that are listed on the issue in question. This ensures that work isn't being duplicated and communicating your plan early also generally leads to better patches.
1. Fork the desired repo, develop and test your code changes.
1. Ensure that your code adheres to the existing style in the sample to which you are contributing.
1. Ensure that your code has an appropriate set of unit tests that all pass.
1. Submit a pull request.

## Code style

Contributions should conform to the coding style and conventions in the existing codebase. Use [.clang-format](.clang-format) for some conventions; examine the source-code for others.

## Requirements for Contributors

If you plan to contribute a patch, you need to build SGParser from a git checkout, which has further requirements:
- [CMake](https://cmake.org) v3.13.0 or newer
- one of the following C++ compilers with [C++17 support](https://en.cppreference.com/w/Template:cpp/compiler_support/17):
    - [GCC](https://gcc.gnu.org/) v10 or newer
    - [Clang](https://clang.llvm.org/) v11 or newer
    - [Microsoft Visual C++](https://visualstudio.microsoft.com/visual-cpp-build-tools/) 2019 or newer
