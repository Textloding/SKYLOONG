$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$halHeader = Join-Path $repoRoot "main\include\hal.h"
$halSource = Join-Path $repoRoot "main\hal.cpp"
$mainSource = Join-Path $repoRoot "main\main.cpp"

function Read-Text($path) {
    return [System.IO.File]::ReadAllText($path)
}

function Assert-Contains($text, $pattern, $message) {
    if ($text -notmatch $pattern) {
        throw $message
    }
}

$header = Read-Text $halHeader
$source = Read-Text $halSource
$main = Read-Text $mainSource

Assert-Contains $source 'server\.on\("/health"' "Web server must expose a tiny /health endpoint for LAN diagnostics."
Assert-Contains $source 'WiFi\.setSleep\(false\)' "Web server mode must disable Wi-Fi sleep to avoid slow or unreachable LAN access."
Assert-Contains $source 'WiFi\.setAutoReconnect\(true\)' "Web server mode must enable STA auto reconnect."
Assert-Contains $source 'webserver_last_alive_ms' "Web server task must publish a heartbeat timestamp."
Assert-Contains $source 'uxTaskGetStackHighWaterMark\(hal\.webserver_task\)' "/health or /info must expose webserver stack headroom."
Assert-Contains $source 'BaseType_t\s+webserver_task_status\s*=\s*xTaskCreatePinnedToCore' "Web server task creation must be checked instead of assuming success."
Assert-Contains $source 'webserver_task_status\s*!=\s*pdPASS' "Web server must handle task creation failure."
Assert-Contains $source 'HTTP server task create failed' "Web server task creation failures must leave serial diagnostics."
Assert-Contains $source '"webserver",\s*4096' "Web server task stack must fit the screen's limited internal heap."
Assert-Contains $header 'webserver_last_alive_ms' "HAL must expose webserver heartbeat state."
Assert-Contains $main 'task_webserver_autostart' "Firmware must auto-start the management server in the background when saved Wi-Fi exists."
Assert-Contains $main 'WiFiMgr\.autoConnectSaved\s*\(' "Management server autostart must reconnect to saved Wi-Fi."
Assert-Contains $main 'hal\.start_webserver\s*\(' "Management server autostart must launch the WebServer."

Write-Host "Web server reliability source checks passed."
