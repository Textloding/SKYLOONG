$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot

function Read-RepoFile([string]$relativePath) {
    Get-Content -Raw -Encoding UTF8 -Path (Join-Path $repoRoot $relativePath)
}

function Assert-Contains([string]$source, [string]$pattern, [string]$message) {
    if ($source -notmatch $pattern) {
        throw $message
    }
}

function Assert-NotContains([string]$source, [string]$pattern, [string]$message) {
    if ($source -match $pattern) {
        throw $message
    }
}

$halHeader = Read-RepoFile "main\include\hal.h"
$halSource = Read-RepoFile "main\hal.cpp"
$protocol = Read-RepoFile "main\protocol.cpp"
$systemTask = [regex]::Match($halSource, '(?s)static void task_systemctl\(.*?(?=static bool isVideoFitValueValid)').Value
$refreshTime = [regex]::Match($halSource, '(?s)bool HAL::refreshTime\(\).*?(?=void HAL::getTime\(\))').Value
$getTime = [regex]::Match($halSource, '(?s)void HAL::getTime\(\).*?(?=uint8_t HAL::getDoW)').Value
$setTime = [regex]::Match($halSource, '(?s)bool HAL::setTime\(.*?(?=/\*-------- NTP)').Value
$handleTime = [regex]::Match($halSource, '(?s)void handleTime\(\).*?(?=////////////////////////////////)').Value

Assert-Contains $halHeader '_rtc_mutex' "HAL must own a dedicated mutex for the bit-banged DS1302 bus."
Assert-Contains $halHeader '_systemctl_task' "HAL must retain the system task handle for a synchronous RTC fallback."
Assert-Contains $halHeader '_rtc_fallback_to_system' "HAL must remember when a failed RTC write requires the system-clock fallback."
Assert-Contains $halHeader 'bool\s+refreshTime\(\)' "HAL must expose a checked RTC snapshot refresh operation."
Assert-Contains $halHeader 'bool\s+setTime\(' "RTC writes must report whether read-back verification succeeded."
Assert-Contains $halSource '_rtc_mutex\s*=\s*xSemaphoreCreateMutex\(\)' "HAL initialization must create the RTC mutex before concurrent access begins."
Assert-Contains $systemTask 'case EVENT_GET_TIME:\s*hal\.refreshTime\(\)' "The system task must update time through the serialized checked path."
Assert-NotContains $systemTask 'DS1302_getDateTime\(&hal\.rtc,\s*&hal\.datetime\)' "An invalid RTC read must never clear the shared display snapshot directly."
Assert-Contains $refreshTime '(?s)DS1302_DateTime\s+snapshot\s*=\s*\{\s*\}.*?_rtc_fallback_to_system.*?rtc_lock\(.*?DS1302_getDateTime\(&rtc,\s*&snapshot\).*?if\s*\(rtc_ok\).*?datetime\s*=\s*snapshot' "RTC reads must validate a local snapshot before publishing it."
Assert-Contains $refreshTime '(?s)time\(NULL\).*?gmtime_r.*?datetime\s*=' "A valid system clock must backstop a failed RTC read."
Assert-Contains $getTime 'RTC_READ_INTERVAL_MS' "Home polling must be throttled instead of occupying the RTC bus continuously."
Assert-Contains $getTime '(?s)_systemctl_task\s*==\s*NULL.*?refreshTime\(\)' "Time reads must still work if the system task could not be created."
Assert-Contains $halSource 'rtcDateTimeMatches\(' "RTC read-back verification must compare the actual value with the requested time."
Assert-Contains $setTime '(?s)rtc_lock\(.*?DS1302_setDateTime.*?DS1302_getDateTime.*?verified_ok\s*=\s*read_ok\s*&&\s*rtcDateTimeMatches\(dt,\s*verified\).*?_rtc_fallback_to_system\.store\(!verified_ok\).*?datetime\s*=' "RTC writes must compare read-back with the target and retain the system fallback after a mismatch."
Assert-Contains $handleTime '(?s)settimeofday\(.*?gmtime_r.*?hal\.setTime' "Browser time sync must update the system UTC clock before writing local RTC time."
Assert-NotContains $handleTime 'localtime_r' "Browser time sync must not apply the host timezone a second time."
Assert-Contains $protocol '(?s)if\s*\(hal\.time_sync\).*?hal\.refreshTime\(\).*?hal\.datetime' "Protocol time sync must read a completed snapshot before sending it."

Write-Host "Time resilience source checks passed."
