# Filename:  build.ps1
# Content:   Build script
# Provided AS IS under MIT License; see LICENSE file in root folder.
#
# It builds parser libraries and sgyacc executable according to specified architecture
# and cornifuration.

[CmdletBinding()]
param (
  [string]
  [Alias('arch')]
  $Architecture = 'x64',

  [string]
  [Alias('config')]
  $Configuration = 'Debug',

  [string]
  $CMake = 'cmake'
)

begin {
  $InformationPreference = 'SilentlyContinue'

  $ProjectDir = $PSScriptRoot

  if ($Architecture.ToLower() -eq "win32") {
    $ArchitectureDirName = 'x86'
  } else {
    $ArchitectureDirName = $Architecture
  }

  if ($Configuration.ToLower() -eq 'debug') {
    $Parallel = 1
  } else {
    $Parallel = 4
  }

  $CMakeOutputDir = Join-Path (Join-Path $PSScriptRoot "build") $ArchitectureDirName

  Write-Information (@(
    "architecture:     $Architecture"
    "configuration:    $Configuration"
    "project dir:      $ProjectDir"
    "cmake output:     $CMakeOutputDir"
    "architecture dir: $ArchitectureDirName"
  ) | Out-String)
}

process {
  & $CMake -S $ProjectDir -B $CMakeOutputDir "-DCMAKE_BUILD_TYPE=$Configuration" -A $Architecture
  & $CMake --build $CMakeOutputDir --config $Configuration --parallel $Parallel
}
