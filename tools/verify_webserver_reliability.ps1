$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$halHeader = Join-Path $repoRoot "main\include\hal.h"
$halSource = Join-Path $repoRoot "main\hal.cpp"
$mainSource = Join-Path $repoRoot "main\main.cpp"
$appManagerSource = Join-Path $repoRoot "main\AppManagerLite.cpp"
$lvglConfig = Join-Path $repoRoot "components\lvgl\lv_conf.h"
$sdkconfigPath = Join-Path $repoRoot "sdkconfig"

function Read-Text($path) {
    return [System.IO.File]::ReadAllText($path)
}

function Assert-Contains($text, $pattern, $message) {
    if ($text -notmatch $pattern) {
        throw $message
    }
}

function Assert-LvglImageCacheSmall($text) {
    $match = [regex]::Match($text, '#define\s+LV_IMG_CACHE_DEF_SIZE\s+(\d+)\b')
    if (-not $match.Success) {
        throw "LVGL image cache size must be explicitly configured."
    }
    $value = [int]$match.Groups[1].Value
    if ($value -lt 1 -or $value -gt 4) {
        throw "LVGL image cache must be 1-4 entries; 0 causes repeated decoder churn and huge values starve heap."
    }
}

$header = Read-Text $halHeader
$source = Read-Text $halSource
$main = Read-Text $mainSource
$appManager = Read-Text $appManagerSource
$lvgl = Read-Text $lvglConfig
$sdkconfig = Read-Text $sdkconfigPath

Assert-Contains $source 'server\.on\("/health"' "Web server must expose a tiny /health endpoint for LAN diagnostics."
Assert-Contains $source '#include\s+"esp_heap_caps\.h"' "Web diagnostics must include esp_heap_caps.h so /health can report internal and PSRAM heap pressure."
Assert-Contains $source 'WiFi\.setSleep\(false\)' "Web server mode must disable Wi-Fi sleep to avoid slow or unreachable LAN access."
Assert-Contains $source 'WiFi\.setAutoReconnect\(true\)' "Web server mode must enable STA auto reconnect."
Assert-Contains $source 'webserver_last_alive_ms' "Web server task must publish a heartbeat timestamp."
Assert-Contains $source '"current_app"' "/health must expose the current app id so black-screen or heavy app issues can be diagnosed over LAN."
Assert-Contains $source '"pet_keypress_seq"' "/health must expose the pet keypress counter for keyboard interaction diagnostics."
Assert-Contains $source 'uxTaskGetStackHighWaterMark\(hal\.webserver_task\)' "/health or /info must expose webserver stack headroom."
Assert-Contains $source '"heap_internal_free"' "/health or /info must expose internal heap headroom for LAN page debugging."
Assert-Contains $source '"heap_largest_internal"' "/health or /info must expose largest internal heap block for LAN page debugging."
Assert-Contains $source '"heap_spiram_free"' "/health or /info must expose PSRAM headroom for LAN page debugging."
Assert-Contains $source '"heap_largest_spiram"' "/health or /info must expose largest PSRAM block for LAN page debugging."
Assert-Contains $source 'BaseType_t\s+webserver_task_status\s*=\s*xTaskCreatePinnedToCore' "Web server task creation must be checked instead of assuming success."
Assert-Contains $source 'webserver_task_status\s*!=\s*pdPASS' "Web server must handle task creation failure."
Assert-Contains $source 'HTTP server task create failed' "Web server task creation failures must leave serial diagnostics."
Assert-Contains $source '"webserver",\s*4096' "Web server task stack must fit the screen's limited internal heap."
Assert-Contains $source 'sendGzippedAsset' "Large embedded web assets must use chunked fixed-size sending instead of one huge send_P write."
Assert-Contains $source 'MSG_DONTWAIT' "Large embedded web assets must use non-blocking socket writes so a slow browser cannot stall the whole management server."
Assert-Contains $source 'asset send timeout' "Large embedded web asset sending must abort slow connections instead of blocking the WebServer task."
Assert-Contains $source 'Cache-Control' "Versioned embedded web assets should be cacheable so browsers do not repeatedly stress the device."
Assert-Contains $source 'server\.on\("/index\.js"[\s\S]*sendGzippedAsset' "/index.js must use chunked asset sending so the LAN management page can load under low heap."
Assert-Contains $source 'server\.on\("/index\.css"[\s\S]*sendGzippedAsset' "/index.css must use chunked asset sending so the LAN management page can render under low heap."
Assert-Contains $source 'server\.on\("/dog_medium\.png"[\s\S]*sendGzippedAsset' "Pet preview asset must use chunked asset sending."
Assert-LvglImageCacheSmall $lvgl
Assert-Contains $sdkconfig 'CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP=y' "Wi-Fi/LWIP buffers must prefer PSRAM; otherwise large web assets can starve internal RAM."
Assert-Contains $sdkconfig 'CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL=1024' "malloc() should move medium allocations to PSRAM so internal RAM survives LAN resource loads."
Assert-Contains $sdkconfig 'CONFIG_LWIP_MAX_SOCKETS=6' "Management web server should keep socket count small on the memory-constrained screen."
Assert-Contains $sdkconfig 'CONFIG_LWIP_MAX_ACTIVE_TCP=6' "Management web server should keep active TCP count small on the memory-constrained screen."
Assert-Contains $sdkconfig 'CONFIG_LWIP_TCP_SND_BUF_DEFAULT=2880' "TCP send buffers should be sized for stability instead of throughput."
Assert-Contains $sdkconfig 'CONFIG_LWIP_TCP_WND_DEFAULT=2880' "TCP receive windows should be sized for stability instead of throughput."
Assert-Contains $sdkconfig '# CONFIG_LWIP_TCP_QUEUE_OOSEQ is not set' "Out-of-order TCP queueing should be disabled to save RAM for the single-user LAN management page."
Assert-Contains $header 'webserver_last_alive_ms' "HAL must expose webserver heartbeat state."
Assert-Contains $main 'task_webserver_autostart' "Firmware must auto-start the management server in the background when saved Wi-Fi exists."
Assert-Contains $main 'vTaskDelete\(NULL\)' "Web server autostart task must delete itself after startup so LAN responses have enough internal heap."
Assert-Contains $main 'start_webserver_early' "Firmware must attempt to start the management server before heavy app restore consumes internal heap."
Assert-Contains $main 'start_webserver_early\s*\(\s*\)\s*;\s*appHome\.init' "Early management server start must happen before app initialization."
Assert-Contains $main 'WiFiMgr\.autoConnectSaved\s*\(' "Management server autostart must reconnect to saved Wi-Fi."
Assert-Contains $main 'hal\.start_webserver\s*\(' "Management server autostart must launch the WebServer."

Write-Host "Web server reliability source checks passed."
