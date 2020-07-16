# Filename:  run.ps1
# Content:   Parser smoke tests runner
# Provided AS IS under MIT License; see LICENSE file in root folder.

param (
  [string]
  $Parser,

  [string]
  $GrammarPath = '..',

  [string]
  [Alias('arch')]
  $Architecture = 'x64',

  [string]
  [Alias('config')]
  $Configuration = 'Debug'
)

begin {
  if ($Architecture.ToLower() -eq "win32") {
    $ArchitectureDirName = 'x86'
  } else {
    $ArchitectureDirName = $Architecture
  }

  if (-not $Parser) {
    $Parser = Join-Path $PSScriptRoot "../../build/$ArchitectureDirName/SGYacc/$Configuration/SGYacc.exe"
  }
}

process {
  $ErrorActionPreference = 'Stop'
  
  Set-Location $PSScriptRoot

  $Parser = Resolve-Path $Parser
  $GrammarPath = Resolve-Path $GrammarPath

  $Grammars = Get-Content -Path "grammar.lst" |
    ForEach-Object { ([string]$_).Trim() } |
    Where-Object -FilterScript  { $_ -and -not ([string]$_).StartsWith("#") }

  $Grammars | ForEach-Object {
    $Grammar = $_
    $TestPath = Join-Path $PSScriptRoot $Grammar
    $SnapshotDir = Join-Path $TestPath "Snapshot"
    $OutputDir = Join-Path $TestPath "Output"

    if ( -not (Test-Path $SnapshotDir)) {
      Write-Error "Failed to locate $SnapshotDir directory"
      break
    }

    New-Item @($SnapshotDir, $OutputDir) -ItemType Directory -Force | Out-Null

    Set-Location $GrammarPath

    $OutputDir = (Resolve-Path $OutputDir -Relative) -replace "\\", "/"

    Write-Progress -Activity $Grammar -Status "Generating"

    $Result = & $Parser $Grammar `
      -cd +f:$OutputDir/"CanonicalData.txt" `
      -dfa +f:$OutputDir/"StaticDFA.h" `
      -nontermenum +f:$OutputDir/"NonTermEnum.h" `
      -prodenum +f:$OutputDir/"ProdEnum.h" `
      -pt +f:$OutputDir/"StaticParseTable.h" `
      -rf +f:$OutputDir/"ReduceFunction.h" `
      -termenum +f:$OutputDir/"TermEnum.h" `
      | Out-String

    if (!$Result.Contains("0 error(s)")) {
      Write-Error "Failed to parse $Grammar`n$Result"
      return
    }

    $Result = & $Parser $Grammar `
      -enumclasses `
      -enumstrings `
      -nontermenum +f:$OutputDir/"NonTermEnumClsStr.h" `
      -prodenum +f:$OutputDir/"ProdEnumClsStr.h" `
      -rf +f:$OutputDir/"ReduceFunctionClsStr.h" `
      -termenum +f:$OutputDir/"TermEnumClsStr.h" `
      | Out-String

    if (!$Result.Contains("0 error(s)")) {
      Write-Error "Failed to parse $Grammar`n$Result"
      return
    }

    Write-Progress -Activity $Grammar -Status "Comparing"

    $DifferenceCount = 0
    
    Get-ChildItem $SnapshotDir | ForEach-Object {
      $Difference = Compare-Object (Get-Content (Join-Path $SnapshotDir $_.Name)) (Get-Content (Join-Path $OutputDir $_.Name))
      if ($Difference) {
        $DifferenceCount++
        Write-Error "Difference found for $Grammar in $_`n$(Out-String -InputObject $Difference)"
      }
    }

    if ($DifferenceCount -eq 0) {
      Remove-Item $OutputDir -Recurse -Force
    }
  }
}
