@rem Filename:  build.bat
@rem Content:   Build script wrapper
@rem Provided AS IS under MIT License; see LICENSE file in root folder.
@rem
@rem It runs the build.ps1 script and passes all the arguments to it.

@echo off

powershell ^
  -NoProfile ^
  -ExecutionPolicy Bypass ^
  -File "%~pdn0.ps1" %*
