$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$wifiManagerHeader = Join-Path $repoRoot "main\include\WiFiManager.h"
$wifiManagerSource = Join-Path $repoRoot "main\WiFiManager.cpp"
$qrcodeSource = Join-Path $repoRoot "main\apps\qrcode.cpp"
$weatherSource = Join-Path $repoRoot "main\apps\Weather.cpp"
$sysinfoSource = Join-Path $repoRoot "main\apps\Sysinfo.cpp"

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

$wifiHeader = Read-Text $wifiManagerHeader
$wifiSource = Read-Text $wifiManagerSource
$qrcode = Read-Text $qrcodeSource
$weather = Read-Text $weatherSource
$sysinfo = Read-Text $sysinfoSource

Assert-Contains $wifiHeader 'autoConnectSaved\s*\(' "WiFiManager.h must expose autoConnectSaved()."
Assert-Contains $wifiSource 'WiFiManager::autoConnectSaved\s*\(' "WiFiManager.cpp must implement autoConnectSaved()."
Assert-Contains $wifiSource 'WiFi\.scanNetworks\s*\(' "autoConnectSaved() must scan available networks."
Assert-Contains $wifiSource 'WiFi\.begin\s*\(\s*ssid\.c_str\(\)\s*,\s*pass\.c_str\(\)\s*\)' "autoConnectSaved() must connect with saved SSID/password."
Assert-Contains $wifiSource 'WiFi\.begin\s*\(\s*ssid\.c_str\(\)\s*\)' "autoConnectSaved() must support saved open networks."
Assert-Contains $wifiSource 'WiFi\.scanDelete\s*\(' "autoConnectSaved() must release scan results."
Assert-Contains $wifiSource 'WiFiTable\[i\]\[0\]\s*!=\s*""' "WiFiManager::save() must persist entries by non-empty SSID."
Assert-NotContains $wifiSource 'WiFiTable\[i\]\[1\]\.length\(\)\s*>=\s*8' "WiFiManager::save() must not drop open networks."
Assert-Contains $wifiSource 'WiFiCount\s*<\s*WIFI_SAVE_MAX' "WiFiManager::init() must cap saved networks at WIFI_SAVE_MAX."
Assert-Contains $wifiSource 'WiFiMgr",\s*"auto connect' "auto-connect attempts should leave serial diagnostics."

$autoPaths = @{
    "qrcode.cpp" = $qrcode
    "Weather.cpp" = $weather
    "Sysinfo.cpp" = $sysinfo
}

foreach ($item in $autoPaths.GetEnumerator()) {
    Assert-Contains $item.Value 'WiFiMgr\.autoConnectSaved\s*\(' "$($item.Key) must use WiFiMgr.autoConnectSaved() for automatic reconnect."
}

Write-Host "WiFi pipeline source checks passed."
