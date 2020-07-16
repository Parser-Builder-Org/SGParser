# Filename:  generate.ps1
# Content:   Script that generates calculator parser files
# Provided AS IS under MIT License; see LICENSE file in root folder.
#
# It generates files required by the calculator sample.
# It will create directory specified as the output for the files if it doesn't exist.

param (
  # Path to the grammar file
  [parameter(Position=0)]
  [string]
  $Grammar = 'src/Calc.gr',

  # Path to parser sgyacc executable
  [string]
  $Parser,

  # Location where to place generated files
  [string]
  $OutputDir = 'generated',

  [string]
  [Alias('arch')]
  $Architecture = 'x64',

  [string]
  [Alias('config')]
  $Configuration = 'Debug'
)

$ErrorActionPreference = 'Stop'

if ($Architecture.ToLower() -eq "win32") {
  $ArchitectureDirName = 'x86'
} else {
  $ArchitectureDirName = $Architecture
}

if (-not $Parser) {
  $Parser = Join-Path $PSScriptRoot "../../build/$ArchitectureDirName/SGYacc/$Configuration/sgyacc.exe" -Resolve
}

$Grammar = Join-Path $PSScriptRoot $Grammar -Resolve
$OutputDir = Join-Path $PSScriptRoot $OutputDir -Resolve

$OutputDir = New-Item @($OutputDir) -ItemType Directory -Force

$OutputDir = (Resolve-Path $OutputDir -Relative) -replace "\\", "/"
$Grammar = (Resolve-Path  $Grammar -Relative) -replace "\\", "/"

& $Parser $Grammar `
  -lalr `
  -enumclasses `
  -enumstrings `
  -dfa +f:$OutputDir/"DFA.h" +classname:"CalcDFA" `
  -prodenum +f:$OutputDir/"ProdEnum.h" `
  -pt +f:$OutputDir/"ParseTable.h" +classname:"CalcParseTable" `
  | Out-Default
