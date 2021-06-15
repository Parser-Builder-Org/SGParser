@echo off

rem Filename:  generate_rus.bat
rem Content:   Wrapper for script that generates calculator parser files for grammar that uses Russian language.
rem Provided AS IS under MIT License; see LICENSE file in root folder.

"%~pd0generate.bat" src/Calc_rus.gr %*
