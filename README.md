<!-- 
Filename:  README.md
Content:   Readme for SGParser project
Provided AS IS under MIT License; see LICENSE file in root folder.
-->
# Simple Grammar Parser

SGParser is a parser generator and a C++ parsing library, which can be used to parse programming languages,
expressions, and other complex text files. To use it you would take several steps:
 1. Write a grammar file for the syntax you need to parse.
 2. Use the command line 'sgyacc' parser generator tool to create parse tables.
 3. Include the Parser in your C++ project. You will use the DFATokenizer and Parser classes, along with the generated parse tables to handle the parsing.
 4. Implement the ParseHandler interface; its Reduce virtual function will be called whenever a production is reduced. Here you have access to a stack of tokens and can add all of your grammar rule processing logic.

 Please see
 [Calculator sample](samples/calc/#simple-calc) source code for more details.

SGParser implements a corrected LALR “Look-Ahead Left-to-right, Rightmost derivation” parser with backtracking support.
The parser is “corrected” in that it can detect and resolve some LALR conflicts automatically.
Backtracking refers to the fact that the parser has a conceptual ability to go back and re-parse productions it encountered in the past, useful
in case the symbol is defined later on in the grammar. Doing so would require extra user-side work and is typically not necessary.

Some reasons you might want to use SGParser instead of 'yacc' include:
 * It's written in C++, so you can benefit from template customization of parse stack elements, etc.
 * You may find stand-alone grammar files cleaner and easier to write.
 * Compact LR(1) parse tables will not suffer from reduce/reduce conflicts related to LALR table merging of states.
 * You can get nice conflict reports and canonical data reports with '-cr' and '-cd' options to help debug the grammar.
 * Support for multiple starting productions, user-controlled reductions (to handle C style type-name/identifier conflict), as well as user-controlled error recovery features described in the [doc](doc/#doc) folder.


## Repo Structure
```
.
├── build.sh                                  # Build debug/release version of 64-bit SGYacc
├── build.bat                                 # Build script wrapper
├── build.ps1                                 # Build debug/release version of 32/64-bit SGYacc with MSVC
├── CMakeLists.txt                            # Root CMake project file
├── CONTRIBUTING.md                           # Information about how to contribute to Parser project
├── LICENSE                                   # MIT License
├── README.md                                 # This file
├── doc                                       # Folder for Parser-related documents
│   └── ...
├── prebuilt                                  # Contains prebuilt binaries
│   └── ...
├── projects                                  # Project files for various build systems
│   ├── cmake                                 # CMake toolchain files for building Parser, Parser Generator and SGYacc
│   │   └── ...
│   └── vs2019                
│       ├── SGParser.sln                      # Solution for Visual Studio 2019
│       └── ...
│           └── ...
├── samples                                   # Samples that show how to use the Parser
│   └── calc                                  # Simple calculator
│       └── ...
├── src                                       # Source code files
│   ├── Parser                                # Parser source and public headers, used by Parser Generator, SGYacc and sample applications
│   │   ├── Kernel                            # Helper classes and functions
│   │   └── ...
│   ├── ParserGen                             # Parser Generator source and public headers, used by SGYacc
│   │   └── ...
│   └── SGYacc                                # SGYacc command-line application
│       └── ...
└── tests                                     # Tests and test-related files
    ├── Calc.gr                               # Sample calculator grammar
    ├── CPre.ycc                              # Grammar for C preprocessor
    ├── CmdLineParser.gr                      # Grammar describing SGYacc command-line options (required by SGYacc)
    └── smoke                                 # Smoke tests related files
        ├── Calc.gr                           # Files for calculator grammar testing
        │   └── Snapshot            
        │       └── ...                       # Pre-generated etalon files for C.ycc grammar
        ├── CmdLineParser.gr                  # Files for CmdLineParser.gr grammar testing
        │   └── Snapshot
        │       └── ...                       # Pre-generated etalon files for CmdLineParser.gr grammar
        ├── grammar.lst                       # Grammar list for smoke test runner
        ├── README.md                         # Smoke tests documentation
        ├── run.bat                           # Smoke test runner main script for 32/64-bit Microsoft Windows build
        ├── run.ps1                           # Smoke test runner PowerShell script (used by run.bat)
        └── run.sh                            # Smoke test runner Bash script
```

## Build

### Microsoft Visual Studio

Use Visual Studio solution file [projects\vs2019\SGParser.sln](projects/vs2019/SGParser.sln) to build complete set of components.
* Use project file [projects\vs2019\Parser\Parser.vcxproj](projects/vs2019/Parser/Parser.vcxproj) to build only Parser library.
* Use project file [projects\vs2019\ParserGen\ParserGen.vcxproj](projects/vs2019/ParserGen/ParserGen.vcxproj) to build Parser and Parser Generator libraries.
* Use project file [projects\vs2019\SGYacc\SGYacc.vcxproj](projects/vs2019/SGYacc/SGYacc.vcxproj) to build Parser library, Parser Generator library and the lexical analyzer *SGYacc*.

### CMake

There are a bunch of scripts to build the parser binaries by one command.

#### On Microsoft Windows

Run one of the following commands to build for the x64 debug, x64 release, win32 debug, and win32 release targets, respectively.
```bat
build
build --config release
build --config debug --arch win32
build --config release --arch win32
```
This will generate CMake-based solutions and executables within the build folder.

The release binaries get placed into the following directories correspondingly. For debug, please replace the 'Release' subdirectory with 'Debug'.

- On x64 platform:
`build\x64\src\Parser\Release\parser.lib`\
`build\x64\src\ParserGen\Release\parsergen.lib`\
`build\x64\SGYacc\Release\sgyacc.exe`

- On win32 platform:
`build\x86\src\Parser\Release\parser.lib`\
`build\x86\src\Parser\Release\parsergen.lib`\
`build\x86\SGYacc\Release\sgyacc.exe`

#### On Linux and macOS

Use commands shown below to build required versions of the parser executables. Please note that only 64-bit builds are supported.
```sh
./build.sh
./build.sh --config debug
./build.sh --config release
```

The binaries get placed into the following directories correspondingly.

- Debug versions:
`build/x64/src/Parser/libparser.a`\
`build/x64/src/ParserGen/libparsergen.a`\
`build/x64/SGYacc/Debug/sgyacc`

- Relase versions:
`build/x64/src/Parser/libparser.a`\
`build/x64/src/ParserGen/libparsergen.a`\
`build/x64/SGYacc/Release/sgyacc`

## Prebuilt binaries

We provide a set of prebuilt release versions of `sgyacc` executables for the following platforms:
- Microsoft Windows 10 64-bit
- Microsoft Windows 10 32-bit
- Ubuntu 18.04
- macOS 10.15.5

They can be found in the repository in the [prebuilt](prebuilt/#prebuilt) directory.
They can be run directly from directories they reside in.

```bat
prebuit\windows_10_x64\sgyacc.exe
```
```bat
prebuit\windows_10_x86\sgyacc.exe
```
```sh
./prebuilt/ubuntu_18.04/sgyacc
```
```sh
./prebuilt/macos_10.15.5/sgyacc
```

## Usage

To check a grammar use simple command that runs `sgyacc` against a file with the grammar definition.
```sh
sgyacc grammar.gr
```

Notice that the tool supports only forward slash symbol in file paths.

To generate the parse table, DFA (deterministic finite automata) table, and production tokens enumeration files please use flags shown below.
```sh
sgyacc grammar -dfa -pt -prodenum
```

To see the complete set of options use any of the following commands
```sh
sgyacc
sgyacc -?
```

## Samples

The repository contains examples that illustrate how to use the parser.

[See samples](samples/#samples)

## Tests

The repository contains tests for the parser.

[See tests](tests/#parser-tests)

## License and Contributing

This source is released AS IS under the [MIT](https://choosealicense.com/licenses/mit/) license.

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update the tests as appropriate.

## Acknowledgments

This library was originally developed by Michael Antonov and Brendan Iribe at Scaleform and transferred to Status Games as a part of internal programming language research work.

The grammar file format was inspired by Visual Parse++ syntax, which was a parsing tool in early 2000s,
so we would like to thank William Donahue for his work on it.
You can read about the grammar file format here:
["Parsing with Sandstone's Visual Parse++"](https://www.researchgate.net/publication/319851057_Parsing_with_Sandstone's_Visual_Parse).

We'd also like to thank Marshall Cline, Andrii Kondratiev, Ed Yablonski, and Eugene Medvedenko for their contribution to modernizing the code for open-source.

For additional information about handling grammar file starting productions, etc. please see [doc](doc/#doc).
