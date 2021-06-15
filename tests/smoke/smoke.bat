@echo off

rem Filename:  smoke.bat
rem Content:   Parser smoke tests runner wrapper.
rem Provided AS IS under MIT License; see LICENSE file in root folder.

rem Run the `smoke.ps1` script from Powershell and pass all the arguments to it.
powershell ^
  -NoProfile ^
  -ExecutionPolicy Bypass ^
  -File "%~pdn0.ps1" %*
