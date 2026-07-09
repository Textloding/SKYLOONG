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

Assert-Contains $web 'dirtyInfo:\s*new Set\(\)' "Web UI must track unsaved device info fields."
Assert-Contains $web 'function\s+mergeInfoFromDevice\s*\(' "Web UI must merge device info without overwriting dirty fields."
Assert-Contains $web 'state\.info\s*=\s*mergeInfoFromDevice\(info\)' "refreshAll() must preserve unsaved device info edits during polling."
Assert-NotContains $web 'state\.info\s*=\s*normalizeInfo\(info\)' "refreshAll() must not blindly replace local device info edits."
Assert-Contains $web 'function\s+markInfoDirty\s*\(' "Web UI must expose a helper to mark dirty device info fields."

Assert-Contains $web 'dirtyAppCfg:\s*new Set\(\)' "Web UI must track unsaved app config fields."
Assert-Contains $web 'function\s+mergeAppCfgFromDevice\s*\(' "Web UI must merge device config without overwriting dirty fields."
Assert-Contains $web 'state\.appCfg\s*=\s*mergeAppCfgFromDevice\(appCfg\)' "refreshAll() must preserve unsaved app config edits during polling."
Assert-NotContains $web 'state\.appCfg\s*=\s*normalizeAppCfg\(appCfg\)' "refreshAll() must not blindly replace local app config edits."
Assert-Contains $web 'function\s+isWeatherEditing\s*\(' "Weather config must detect active edits before accepting polled device values."
Assert-Contains $web 'collectWeatherDraftFromDom\(\)' "Weather config must preserve current DOM values while the user is editing."
Assert-Contains $web 'weatherKeyDrafts:\s*\{\}' "Weather keys must keep per-provider drafts while switching sources."

Assert-Contains $web 'input\.addEventListener\("input",[\s\S]*?state\.dirtyAppCfg\.add\(key\)[\s\S]*?state\.appCfg\s*=\s*normalizeAppCfg' "Manual app config edits must mark the edited field dirty before updating state."
Assert-Contains $web 'state\.dirtyAppCfg\.add\("city"\)[\s\S]*?state\.appCfg\s*=\s*normalizeAppCfg\(\{\s*\.\.\.state\.appCfg,\s*city:\s*btn\.dataset\.cityPick\s*\}\)' "Weather city shortcut buttons must mark city dirty."
Assert-Contains $web 'state\.dirtyAppCfg\.add\("city"\)[\s\S]*?state\.dirtyAppCfg\.add\("weather_lat"\)[\s\S]*?state\.dirtyAppCfg\.add\("weather_lon"\)[\s\S]*?state\.appCfg\s*=\s*normalizeAppCfg' "Weather auto-detect must mark city and coordinates dirty."
Assert-Contains $web 'const\s+saved\s*=\s*await\s+runAction\("appcfg"[\s\S]*?if\s*\(saved\)\s*\{[\s\S]*?state\.dirtyAppCfg\.clear\(\)' "Dirty app config fields must clear only after a successful save."

Write-Host "web dirty-state source checks passed."
