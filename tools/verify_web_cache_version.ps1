param(
    [string]$ExpectedVersion = "modern-20260707j",
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

$ErrorActionPreference = "Stop"

function Read-Text([string]$relativePath) {
    [System.IO.File]::ReadAllText((Join-Path $RepoRoot $relativePath))
}

function Get-AssetVersions([string]$html) {
    $matches = [regex]::Matches($html, '<(?:link|script)[^>]+(?:href|src)="/index\.(?:css|js)\?v=([^"]+)"')
    $matches | ForEach-Object { $_.Groups[1].Value }
}

$webNewVersions = @(Get-AssetVersions (Read-Text "web_new\index.html"))
$webVersions = @(Get-AssetVersions (Read-Text "web\index.html"))

if ($webNewVersions.Count -ne 2) {
    throw "web_new/index.html must cache-bust both index.css and index.js."
}

if ($webVersions.Count -ne 2) {
    throw "web/index.html must cache-bust both index.css and index.js."
}

foreach ($version in @($webNewVersions + $webVersions)) {
    if ($version -ne $ExpectedVersion) {
        throw "Web cache version must be $ExpectedVersion, got $version."
    }
}

Write-Host "web cache version checks passed."
