[CmdletBinding()]
param(
    [string]$Project = "KuboEngine.sln",
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",
    [ValidateSet("x64", "Win32")]
    [string]$Platform = "x64",
    [switch]$Restore
)

$ErrorActionPreference = "Stop"

function Resolve-MSBuildPath {
    $candidates = @(
        "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    throw "MSBuild.exe was not found in the expected Visual Studio locations."
}

$root = Split-Path -Parent $PSScriptRoot
$projectPath = if ([System.IO.Path]::IsPathRooted($Project)) {
    $Project
} else {
    Join-Path $root $Project
}

if (-not (Test-Path $projectPath)) {
    throw "Project file not found: $projectPath"
}

$msbuild = Resolve-MSBuildPath

$arguments = @(
    $projectPath
    "/m"
    "/p:Configuration=$Configuration"
    "/p:Platform=$Platform"
)

if ($Restore) {
    $arguments += "/restore"
}

Write-Host "MSBuild: $msbuild"
Write-Host "Project: $projectPath"
Write-Host "Configuration: $Configuration"
Write-Host "Platform: $Platform"

& $msbuild @arguments
$exitCode = $LASTEXITCODE

if ($exitCode -ne 0) {
    throw "MSBuild failed with exit code $exitCode"
}
