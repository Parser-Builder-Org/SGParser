@echo off

rem Filename:  build.bat
rem Content:   Build script wrapper.
rem Provided AS IS under MIT License; see LICENSE file in root folder.

rem Build Parsera and Parser Generator libraries and then build SGYacc executable 
rem and Sample executables according to specified architecture and configuration.
rem Examples of usage:
rem - build with default parameters:               build.bat
rem - build debug version for x86 architecture:    build.bat --arch x86 --config Debug
rem - build release version for x86 architecture:  build.bat --arch x86 --config Release
rem - build debug version for x64 architecture:    build.bat --arch x64 --config Debug
rem - build release version for x64 architecture:  build.bat --arch x64 --config Release

rem Run the `build.ps1` script from PowerShell and pass all the arguments to it.
powershell ^
  -NoProfile ^
  -ExecutionPolicy Bypass ^
  -File "%~pdn0.ps1" %*
