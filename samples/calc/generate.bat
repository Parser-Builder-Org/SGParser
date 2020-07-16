@rem Filename:  generate.bat
@rem Content:   Script wrapper
@rem Provided AS IS under MIT License; see LICENSE file in root folder.

@echo off

powershell ^
  -NoProfile ^
  -ExecutionPolicy Bypass ^
  -File "%~pdn0.ps1" %*
