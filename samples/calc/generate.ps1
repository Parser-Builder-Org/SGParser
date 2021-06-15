# Filename:  generate.ps1
# Content:   PowerShell script that generates Calc parser files.
# Provided AS IS under MIT License; see LICENSE file in root folder.

# Generate parser source files required by the Calc sample.
# Will create a folder specified as the output for the files if it doesn't exist.
# This script should be run after some changes were made in the Calc grammar.
# Can be used directly from PowerShell or from `generate.bat` wrapper.
# Examples of usage from PowerShell:
# - generate by default version of SGYacc:       generate.ps1
# - generate by 32-bit debug version of SGYacc:  generate.ps1 --arch x86 --config Debug
# - generate by 32-bit debug version of SGYacc:  generate.ps1 --arch x86 --config Release
# - generate by 64-bit debug version of SGYacc:  generate.ps1 --arch x64 --config Debug
# - generate by 64-bit debug version of SGYacc:  generate.ps1 --arch x64 --config Release

# List of available command-line options and their default values.
[CmdletBinding()]
param (
  # Path to the grammar file.
  [parameter(Position=0)]
  [string]
  $Grammar = 'src/Calc.gr',

  # Path to SGYacc executable.
  # Will override the configured one if provided (usable for script debugging).
  [string]
  $Parser,

  # Folder where to place generated files.
  [string]
  $OutputDir = 'generated',

  # Generate by 32-bit version of SGYacc by default.
  [string]
  [Alias('arch')]
  $Architecture = 'x86',

  # Generate by Debug version of SGYacc by default.
  [string]
  [Alias('config')]
  $Configuration = 'Debug'
)

# Stop on any error.
$ErrorActionPreference = 'Stop'

# Set architecture-dependent parameters.
if ($Architecture.ToLower() -eq 'x86') {
  $ArchitectureDirName = 'x86'
} elseif ($Architecture.ToLower() -eq 'x64') {
  $ArchitectureDirName = 'x64'
} else {
  Write-Error -Message "Error: Only the following architectures are supported: x86, x64" -Category InvalidArgument
  break
}

# Set configuration-dependent parameters.
if ($Configuration.ToLower() -eq 'debug') {
  $Configuration = 'Debug'
} elseif ($Configuration.ToLower() -eq 'release') {
  $Configuration = 'Release'
} else {
  Write-Error -Message "Error: Only the following configurations are supported: Debug, Release" -Category InvalidArgument
  break
}

# Set parameters-dependent path to SGYacc executable (if not provided).
if (-not $Parser) {
  $Parser = Join-Path $PSScriptRoot "../../build/$ArchitectureDirName/SGYacc/$Configuration/sgyacc.exe" -Resolve
}

# Setup the required folders. 
$Grammar = Join-Path $PSScriptRoot $Grammar -Resolve
$OutputDir = Join-Path $PSScriptRoot $OutputDir -Resolve

$OutputDir = New-Item @($OutputDir) -ItemType Directory -Force

$OutputDir = (Resolve-Path $OutputDir -Relative) -replace "\\", "/"
$Grammar = (Resolve-Path  $Grammar -Relative) -replace "\\", "/"

# Generate calculator parser source files.
& $Parser $Grammar `
  -lalr `
  -enumclasses `
  -enumstrings `
  -namespaces +nsname:Calc `
  -dfa +f:$OutputDir/"DFA.h" +classname:"CalcDFA" `
  -prodenum +f:$OutputDir/"ProdEnum.h" `
  -pt +f:$OutputDir/"ParseTable.h" +classname:"CalcParseTable" `
  | Out-Default
