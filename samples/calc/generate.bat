@echo off

rem Filename:  generate.bat
rem Content:   Wrapper for Calc parser files generator script.
rem Provided AS IS under MIT License; see LICENSE file in root folder.

rem Generate parser source files required by the Calc sample.
rem Will create a folder specified as the output for the files if it doesn't exist.
rem This script should be run after some changes were made in the Calc grammar.
rem Examples of usage:
rem - generate by default version of SGYacc:       generate.bat
rem - generate by 32-bit debug version of SGYacc:  generate.bat --arch x86 --config Debug
rem - generate by 32-bit debug version of SGYacc:  generate.bat --arch x86 --config Release
rem - generate by 64-bit debug version of SGYacc:  generate.bat --arch x64 --config Debug
rem - generate by 64-bit debug version of SGYacc:  generate.bat --arch x64 --config Release

rem Runs the `generate.ps1` script from PowerShell and pass all the arguments to it.
powershell ^
  -NoProfile ^
  -ExecutionPolicy Bypass ^
  -File "%~pdn0.ps1" %*
