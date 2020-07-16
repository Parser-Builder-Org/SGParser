<!-- 
Filename:  README.md
Content:   Readme for SGParser sample projects
Provided AS IS under MIT License; see LICENSE file in root folder.
-->
# Samples

## Build

The build script that builds the parser also builds the samples.

### CMake

#### On Microsoft Windows

To build samples run one of the following commands from the project root folder (depending on configuration and platform you want to build for):
```bat
build --config debug --arch x64
build --config release --arch x64
```

This will build binaries into the following directories correspondingly\
`build\x64\samples\Debug\`\
`build\x64\samples\Release\`

```bat
build --config debug --arch win32
build --config release --arch win32
```

This will build binaries into the following directories correspondingly\
`build\x86\samples\Debug\`\
`build\x86\samples\Release\`

The platform option can be omitted. Then it will build for the x64 platform by default.
```bat
build --config debug
build --config release
```

It is possible to omit the configuration option too.
```bat
build
```
Then it will build a debug version of binaries for x64 platform.

#### On Linux and macOS

To build samples run one of the following commands from the project root folder (depending on configuration and platform you want to build for):
```sh
./build.sh --config debug
./build.sh --config release
```

This will build binaries into the following directories correspondingly\
`build/x64/samples/Debug/`\
`build/x64/samples/Release/`

It is possible to omit the configuration option.
```sh
./build.sh
```
It will build a debug version of binaries for x64 platform.

[Simple calculator](calc/#simple-calculator)

[Back to parser](../../../#simple-grammar-parser)
