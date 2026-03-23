param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$CodexArgs
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
. (Join-Path $scriptDir "Use-GitHubMcpToken.ps1")

$codexCommand = Get-Command codex -ErrorAction Stop
& $codexCommand.Source @CodexArgs
