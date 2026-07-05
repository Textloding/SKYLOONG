param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

$ErrorActionPreference = "Stop"

function Read-Text($path) {
    return [System.IO.File]::ReadAllText($path)
}

function Assert-Contains($text, $pattern, $message) {
    if ($text -notmatch $pattern) {
        throw $message
    }
}

function Assert-NotContains($text, $pattern, $message) {
    if ($text -match $pattern) {
        throw $message
    }
}

$web = Read-Text (Join-Path $RepoRoot "web_new\index.js")

Assert-Contains $web 'dirtyAppCfg:\s*new Set\(\)' "Web UI must track unsaved app config fields."
Assert-Contains $web 'function\s+mergeAppCfgFromDevice\s*\(' "Web UI must merge device config without overwriting dirty fields."
Assert-Contains $web 'state\.appCfg\s*=\s*mergeAppCfgFromDevice\(appCfg\)' "refreshAll() must preserve unsaved app config edits during polling."
Assert-NotContains $web 'state\.appCfg\s*=\s*normalizeAppCfg\(appCfg\)' "refreshAll() must not blindly replace local app config edits."

Assert-Contains $web 'input\.addEventListener\("input",[\s\S]*?state\.dirtyAppCfg\.add\(key\)[\s\S]*?state\.appCfg\s*=\s*normalizeAppCfg' "Manual app config edits must mark the edited field dirty before updating state."
Assert-Contains $web 'state\.dirtyAppCfg\.add\("city"\)[\s\S]*?state\.appCfg\s*=\s*normalizeAppCfg\(\{\s*\.\.\.state\.appCfg,\s*city:\s*btn\.dataset\.cityPick\s*\}\)' "Weather city shortcut buttons must mark city dirty."
Assert-Contains $web 'state\.dirtyAppCfg\.add\("city"\)[\s\S]*?state\.appCfg\s*=\s*normalizeAppCfg\(\{\s*\.\.\.state\.appCfg,\s*city\s*\}\)' "Weather auto-detect must mark city dirty."
Assert-Contains $web 'const\s+saved\s*=\s*await\s+runAction\("appcfg"[\s\S]*?if\s*\(saved\)\s*state\.dirtyAppCfg\.clear\(\)' "Dirty app config fields must clear only after a successful save."

Write-Host "web dirty-state source checks passed."
