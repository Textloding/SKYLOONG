param(
    [string]$ExpectedCssVersion = "modern-20260709c",
    [string]$ExpectedJsVersion = "modern-20260709f",
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

$ErrorActionPreference = "Stop"

function Read-Text([string]$relativePath) {
    [System.IO.File]::ReadAllText((Join-Path $RepoRoot $relativePath))
}

function Get-AssetVersions([string]$html) {
    $matches = [regex]::Matches($html, '<(?:link|script)[^>]+(?:href|src)="/index\.(css|js)\?v=([^"]+)"')
    $versions = @{}
    foreach ($match in $matches) {
        $versions[$match.Groups[1].Value] = $match.Groups[2].Value
    }
    $versions
}

$webNewVersions = Get-AssetVersions (Read-Text "web_new\index.html")
$webVersions = Get-AssetVersions (Read-Text "web\index.html")

if ($webNewVersions.Count -ne 2) {
    throw "web_new/index.html must cache-bust both index.css and index.js."
}

if ($webVersions.Count -ne 2) {
    throw "web/index.html must cache-bust both index.css and index.js."
}

foreach ($entry in @(
    @{ File = "web_new/index.html"; Versions = $webNewVersions },
    @{ File = "web/index.html"; Versions = $webVersions }
)) {
    if ($entry.Versions["css"] -ne $ExpectedCssVersion) {
        throw "$($entry.File) CSS cache version must be $ExpectedCssVersion, got $($entry.Versions["css"])."
    }

    if ($entry.Versions["js"] -ne $ExpectedJsVersion) {
        throw "$($entry.File) JS cache version must be $ExpectedJsVersion, got $($entry.Versions["js"])."
    }
}

Write-Host "web cache version checks passed."
