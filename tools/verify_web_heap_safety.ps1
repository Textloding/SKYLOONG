$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$halPath = Join-Path $repoRoot "main\hal.cpp"
$source = Get-Content -Raw -Path $halPath

$directPrintPattern = 'strncpy\s*\([^;]*cJSON_PrintUnformatted\s*\('
if ($source -match $directPrintPattern) {
    Write-Error "hal.cpp still copies cJSON_PrintUnformatted() directly. Store the printed string and release it with cJSON_free()."
}

if ($source -notmatch 'cJSON_free\s*\(') {
    Write-Error "hal.cpp does not release cJSON_PrintUnformatted() allocations with cJSON_free()."
}

Write-Host "Web JSON heap-safety checks passed."
