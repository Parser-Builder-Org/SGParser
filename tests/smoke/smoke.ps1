# Filename:  smoke.ps1
# Content:   PowerShell version of Parser smoke tests runner.
# Provided AS IS under MIT License; see LICENSE file in root folder.

# Iterates through all grammar files from `grammar.lst`, 
# for each it runs the SGYacc executable and compares output files
# with the expected ones from the corresponding Snapshot folder.

# List of available command-line options and their default values.
param (
  # Path to SGYacc executable.
  # Will override the configured one if provided (usable for script debugging).
  [string]
  $Parser,

  # Path to `grammar.lst` file.
  [string]
  $GrammarPath = '..',

  # Test the x86 build by default.
  [string]
  [Alias('arch')]
  $Architecture = 'x86',

  # Test the Debug version by default.
  [string]
  [Alias('config')]
  $Configuration = 'Debug'
)

begin {
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
    $Parser = Join-Path $PSScriptRoot "../../build/$ArchitectureDirName/SGYacc/$Configuration/SGYacc.exe" -Resolve
  }
}

process {
  # Stop on any error.
  $ErrorActionPreference = 'Stop'
  
  Set-Location $PSScriptRoot

  $Parser = Resolve-Path $Parser
  $GrammarPath = Resolve-Path $GrammarPath

  # Extract a list of grammars to test from `grammar.lst`.
  $Grammars = Get-Content -Path "grammar.lst" |
    ForEach-Object { ([string]$_).Trim() } |
    Where-Object -FilterScript  { $_ -and -not ([string]$_).StartsWith("#") }

  # Iterate through the list of grammar files and test them.
  $Grammars | ForEach-Object {
    $Grammar = $_
    $TestPath = Join-Path $PSScriptRoot $Grammar
    $SnapshotDir = Join-Path $TestPath "Snapshot"
    $OutputDir = Join-Path $TestPath "Output"

    # Check and exit of no snapshot for the current grammar file are available.
    if ( -not (Test-Path $SnapshotDir)) {
      Write-Error "Failed to locate $SnapshotDir directory"
      break
    }

    New-Item @($SnapshotDir, $OutputDir) -ItemType Directory -Force | Out-Null

    Set-Location $GrammarPath

    $OutputDir = (Resolve-Path $OutputDir -Relative) -replace "\\", "/"

    # Indicate progress.
    Write-Progress -Activity $Grammar -Status "Generating"

    # Run the parser and capture the output (a "new" version, with the enum classes).
    $Result = & $Parser $Grammar `
      -cd +f:$OutputDir/"CanonicalData.txt" `
      -dfa +f:$OutputDir/"StaticDFA.h" `
      -nontermenum +f:$OutputDir/"NonTermEnum.h" `
      -prodenum +f:$OutputDir/"ProdEnum.h" `
      -pt +f:$OutputDir/"StaticParseTable.h" `
      -rf +f:$OutputDir/"ReduceFunction.h" `
      -termenum +f:$OutputDir/"TermEnum.h" `
      | Out-String

    # Exit if errors are present.
    if (!$Result.Contains("0 error(s)")) {
      Write-Error "Failed to parse $Grammar`n$Result"
      return
    }

    # Run the parser and capture the output (a "new" version, with the enum classes).
    $Result = & $Parser $Grammar `
      -namespaces +nsname:XC `
      -enumclasses `
      -enumstrings `
      -nontermenum +f:$OutputDir/"NonTermEnumClsStr.h" `
      -prodenum +f:$OutputDir/"ProdEnumClsStr.h" `
      -rf +f:$OutputDir/"ReduceFunctionClsStr.h" `
      -termenum +f:$OutputDir/"TermEnumClsStr.h" `
      | Out-String

    # Exit if errors are present.
    if (!$Result.Contains("0 error(s)")) {
      Write-Error "Failed to parse $Grammar`n$Result"
      return
    }

    # Indicate progress.
    Write-Progress -Activity $Grammar -Status "Comparing"

    $DifferenceCount = 0
    
    # Check all generated files for equality with snapshot files.
    Get-ChildItem $SnapshotDir | ForEach-Object {
      $Difference = Compare-Object (Get-Content (Join-Path $SnapshotDir $_.Name)) (Get-Content (Join-Path $OutputDir $_.Name))
      if ($Difference) {
        $DifferenceCount++
        Write-Error "Difference found for $Grammar in $_`n$(Out-String -InputObject $Difference)"
      }
    }

    # Remove all generated files if tests were successfully passed.
    if ($DifferenceCount -eq 0) {
      Remove-Item $OutputDir -Recurse -Force
    }
  }
}
