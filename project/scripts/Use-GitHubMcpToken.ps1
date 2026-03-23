param(
    [string]$EnvFile = (Join-Path $PSScriptRoot "..\\.env")
)

if (-not (Test-Path $EnvFile)) {
    throw ".env file not found: $EnvFile"
}

$lines = Get-Content $EnvFile | Where-Object {
    $_.Trim() -and -not $_.TrimStart().StartsWith("#")
}

$token = $null

foreach ($line in $lines) {
    if ($line -match '^\s*(GITHUB_PAT_TOKEN|GITHUB_TOKEN|GH_TOKEN)\s*=\s*(.+?)\s*$') {
        $token = $matches[2].Trim()
        break
    }
}

if (-not $token -and $lines.Count -eq 1 -and $lines[0] -notmatch '=') {
    $token = $lines[0].Trim()
}

if (-not $token) {
    throw "No GitHub token entry found in $EnvFile"
}

$env:GITHUB_PAT_TOKEN = $token
Write-Host "Loaded GITHUB_PAT_TOKEN into the current PowerShell session."
