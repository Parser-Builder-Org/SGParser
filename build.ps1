# Filename:  build.ps1
# Content:   PowerShell build script.
# Provided AS IS under MIT License; see LICENSE file in root folder.

# Build Parsera and Parser Generator libraries and then build SGYacc executable 
# and Sample executables according to specified architecture and configuration.
# Can be used directly from PowerShell or from `build.bat` wrapper.
# Examples of usage from PowerShell:
# - build with default parameters:               build.ps1
# - build debug version for x86 architecture:    build.ps1 --arch x86 --config debug
# - build release version for x86 architecture:  build.ps1 --arch x86 --config release
# - build debug version for x64 architecture:    build.ps1 --arch x64 --config debug
# - build release version for x64 architecture:  build.ps1 --arch x64 --config release

# Set minimum required PowerShell version.
#Requires -Version 5.0

# List of available command-line options and their default values.
[CmdletBinding()]
param (
  # Build for x64 architecture by default.
  [string]
  [Alias('arch')]
  [ValidateSet('x64','x86')]
  $Architecture = 'x64',

  # Build in Debug configuration by default.
  [string]
  [Alias('config')]
  [ValidateSet('debug','release')]
  $Configuration = 'debug',

  # Name of CMake executable (usable for script debugging).
  [string]
  $CMake = 'cmake',

  [string]
  [ValidateSet('msvc')] # @todo support clang, wsl-clang etc. 
  $Toolkit = 'msvc',

  # Use Ninja by default (faster builds), use option to disable
  [switch]
  [Alias('nn')]
  $NoNinja
)

begin {
  # Set effect from `Write-Information` command (the informational messages
  # aren't displayed, and the script continues without interruption).
  $InformationPreference = 'SilentlyContinue'
  $ProjectDir = $PSScriptRoot
  $UseNinja = ! $NoNinja

  # Set architecture-dependent parameters.
  if ($Architecture.ToLower() -eq 'x86') {
    $Platform = 'Win32'
    $ArchitectureDirName = 'x86'
  } elseif ($Architecture.ToLower() -eq 'x64') {
    $Platform = 'x64'
    $ArchitectureDirName = 'x64'
  } else {
    Write-Error -Message "Error: Only the following architectures are supported: x86, x64" -Category InvalidArgument
    break
  }

    # Set configuration-dependent parameters.
  if ($Configuration.ToLower() -eq 'debug') {
    $Configuration = 'debug'
  } elseif ($Configuration.ToLower() -eq 'release') {
    $Configuration = 'release'
  } else {
    Write-Error -Message "Error: Only the following configurations are supported: debug, release" -Category InvalidArgument
    break
  }

  # Set parameters-dependent path to `build` folder (resulted binary will be placed there).
  $CMakeOutputDir = Join-Path $PSScriptRoot "build/$ArchitectureDirName-$Toolkit-$Configuration"

  [string] $ConfigParam = "-S ""$ProjectDir"" -B ""$CMakeOutputDir"" -DCMAKE_BUILD_TYPE=$Configuration"
  [string] $BuildParam = "$CMakeOutputDir --config $Configuration"
  if($UseNinja) {
    $ConfigParam += " -G ""Ninja"""  
  } else {
    $ConfigParam += " -A $Platform"
    $Parallel = 4
    $BuildParam += " --parallel $Parallel"
    $ConfigParam += " -DCMAKE_CONFIGURATION_TYPES=""$Configuration"""
  } 

  # Write the configuration parameters to a string (usable for script debugging).
  Write-Information (@(
    "architecture:     $Architecture"
    "platform:         $Platform"
    "configuration:    $Configuration"
    "project dir:      $ProjectDir"
    "cmake output:     $CMakeOutputDir"
    "architecture dir: $ArchitectureDirName"
    "config args:      $ConfigParam"
    "build args:       $BuildParam"
  ) | Out-String)
}

# Build SGParser project with CMake.
process {
  if($UseNinja) {
    # embed environment so cl.exe etc. are available for Ninja 
    & support/build/setupVcEnv.ps1
  }
  Invoke-Expression "$CMake $ConfigParam"
  Invoke-Expression "$CMake --build $BuildParam"
}
