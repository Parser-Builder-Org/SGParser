@rem Filename:  run.bat
@rem Content:   Parser smoke tests runner for 32/64-bit build
@rem Provided AS IS under MIT License; see LICENSE file in root folder.

@echo off

powershell ^
  -NoProfile ^
  -ExecutionPolicy Bypass ^
  -File "%~pdn0.ps1" %*
