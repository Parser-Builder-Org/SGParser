# Filename:  setupVcEnv.ps1
# Content:   PowerShell script for getting msvc environment into current Powershell session
# Provided AS IS under MIT License; see LICENSE file in root folder.

# Set minimum required PowerShell version.
#Requires -Version 5.0

# List of available command-line options and their default values.
# Typical invocations are:
#   `setupVcEnv.ps1` (no param) - embedds default(2019) default tolkit environment.
#   `setupVcEnv.ps1 -VsVer=2019` - embedds 2019 default tolkit environment.
#   `setupVcEnv.ps1 2019` - the same as above
#   `setupVcEnv.ps1 -Toolkit=14.28` - embedds 2019/14.28 toolkit environment.
#   `setupVEnv.ps1 14.28` - the same as above
#
# It's possible to use 2017's toolkit coming with VS 2019:
# `setupVsEnv.ps1 -Toolkit=14.12 -VsVer=2019`
[CmdletBinding()]
param
(
  # Toolkit version in the form "14.28", "14.29" etc.
  # Also, for convinience, it can be VS version (e.g. "2019", in this case VsVer must be omitted).
  # '' means "take default"
  [string]
  $Toolkit = '',

  # Visual Studio version (2015,2017,2019 etc.).
  [string]
  [ValidateSet('2015','2017','2019')]
  $VsVer = ''
)

$VS_DEFAULT_VERSION = 2019

# Returns full path to `vswhere.exe`
function _GetVsWherePath {
  $VSWHERE_LOCATION = [System.Environment]::ExpandEnvironmentVariables("%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe") 
  if( Test-Path -Path $VSWHERE_LOCATION -PathType Leaf ) {
    return $VSWHERE_LOCATION
  } else {
    throw "vswhere does not exist at $VSWHERE_LOCATION"
  }  
}

# Check if given number is suported VS version. 2019 only for now
function IsSupportedVsVersion {
  param ( [string]$ver  )
  # The only active version for now. The script technically could work with VS 2015, 2017
  return ('2015','2017','2019').Contains($ver);
}

# For given toolkit version '14.28'. '14.10' etc. Gives corresponding VS version:
# 14.0 -> 2015
# 14.12 -> 2017
# 14.10 -> 2017
# 14.11 -> 2017
# 14.20 -> 2019
# 14.28 -> 2019
# 14.29 -> 2019
function MapToolkitVersionToVSVersion {
  param( $toolkit )
  if( -not ($toolkit -match '(\d+)\.(\d+)') ) {
    throw "Toolkit '$toolkit' does not match expected form NN.NN"
  }
  $major = [int]$Matches[1]
  $minor = [int]$Matches[2]
  Write-Debug "Parsed-version $major.$minor"
  if ( $major -ne 14 ) {
    throw "Unsupported toolkit version $Toolkit. Only 14.XX versions are supported"
  }

  if ( $minor -eq 0 ) { # 14.0 is 2015
    return 2015
  } elseif ((($minor -ge 10) -and ($minor -lt 20)) -or ( $minor -eq 1 ) ) { # 14.1X are 2017
    return 2017
  } elseif ((($minor -ge 20) -and ($minor -lt 30)) -or ( $minor -eq 2 ) ) { # 14.2X are 2019
    return 2019
  } else {
    throw "Unknown toolkit version $Toolkit. Supported are 14.00..14.29"
  }
}

function GetVisualStudioPath {
  param (
    [string]$ver = $VS_DEFAULT_VERSION
  )
  $vswhere = &_GetVsWherePath
  $reply = Invoke-Expression "& ""$vswhere"" -all -products * -property installationPath"
  $pathd = $reply | ForEach-Object {([System.IO.DirectoryInfo]($_))}
  $path = $pathd.where( { [string](($_).Parent.Name) -eq $ver } ) 
  if( $path.Count -eq 0) {
    throw "Can't find result from output of vswhere where version is $ver. Probably Visual Studio $ver is not installed."
  } elseif( $path.Count -ne 1) {
    Write-Warning "vswhere returned more than one result matching version $ver. Taking the first entry".
  }
  Write-Debug "Visual Studio is installed at $($path[0])"
  return $path[0]
}

# Runs given command in the new CMD.exe instance, takes environment in CMD 
# and copies it back to the current powershell environment
function _embedEnv {
  param ( $command  )
  Write-Debug "Command is $command"

  # run command and call `set` immediately to print environment, capture output to $answer
  $answer = cmd /c "$command && set" 2>&1

  if( $LASTEXITCODE -ne 0 ) {
    $err = "Command $command returned error. Trace: `n"
    $answer | ForEach-Object { 
        $err +="  "+ $_.ToString() + "`n"
    }
    throw $err
  }

  # Set values captured in $answer into current powershell process environment
  $answer | ForEach-Object {
    if ($_ -match "=") {
      $v = $_.split("="); set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
    }
  }  
}

function EmbedVsdev {
  param (
    [string]$vs_ver,
    [string]$toolkit
  )
  
  $vs_root = GetVisualStudioPath $vs_ver

  $VS_DEV_CMD_SUBPATH = '\Common7\Tools\VsDevCmd.bat'
  $vs_tools_cmd = """$vs_root$VS_DEV_CMD_SUBPATH"" -arch=x64 -host_arch=x64"
  if( $toolkit -ne '' ) {
    $vs_tools_cmd += " -vcvars_ver=$toolkit"
  }
  _embedEnv $vs_tools_cmd
  Write-Host "`nVisual Studio $vs_ver[$toolkit] Command Prompt variables set." -ForegroundColor Yellow
}

try {
    if( IsSupportedVsVersion ($Toolkit) ) {
      # $Toolkit is e.g. "2019". In this case it actually should be VsVer.
      if( ($VsVer -ne '') -and ($VsVer -ne $Toolkit) ) {
        throw "Toolkit supported Visual Studio version ($Toolkit), but is inconsistent with VsVer ('$VsVer')"
      }
      # Use default toolkit of given VS version
      $VsVer = $Toolkit
      $Toolkit = ''
    }

    if ( $VsVer -eq '') {
      if( $Toolkit -eq '' ) {
        $VsVer = $VS_DEFAULT_VERSION
      } else {
        $VsVer = MapToolkitVersionToVSVersion $Toolkit 
      }
    }

    Write-Host  "Preparing environment for $VsVer[$Toolkit]" -ForegroundColor Yellow
    EmbedVsdev $VsVer $Toolkit
} catch {
    Write-Host $_.Exception.Message -ForegroundColor Red
}