$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$cacheHeaderPath = Join-Path $repoRoot "main\include\weather_cache.h"
$homeCpp = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\apps\home.cpp")
$weatherCpp = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\apps\Weather.cpp")
$halCpp = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\hal.cpp")

function Assert-Contains($Text, $Pattern, $Message) {
    if ($Text -notmatch $Pattern) {
        throw $Message
    }
}

function Assert-NotContains($Text, $Pattern, $Message) {
    if ($Text -match $Pattern) {
        throw $Message
    }
}

if (!(Test-Path $cacheHeaderPath)) {
    throw "Weather cache readers must share one validated on-disk structure."
}

$cacheHeader = Get-Content -Raw -Encoding UTF8 $cacheHeaderPath
Assert-Contains $cacheHeader 'struct\s+WeatherCacheData' "The persisted weather-cache layout must have one shared definition."
Assert-Contains $cacheHeader 'weatherCacheIsValid' "The shared weather cache must validate string termination and weather-code ranges."
Assert-Contains $cacheHeader 'memchr' "Persisted strings must be checked within their fixed-size fields."
Assert-Contains $cacheHeader 'static_assert\s*\(\s*sizeof\(WeatherCacheData\)\s*==\s*100' "Changing the raw weather-cache layout must fail at build time."

Assert-Contains $homeCpp '#include\s+"weather_cache\.h"' "The space theme must use the shared weather-cache validator."
Assert-Contains $weatherCpp '#include\s+"weather_cache\.h"' "The weather app must use the shared weather-cache validator."
Assert-NotContains $homeCpp 'struct\s+HomeWeatherData' "The space theme must not duplicate the persisted cache layout."
Assert-NotContains $weatherCpp '(?-i:\bWeatherData\b)' "The weather app must use the shared persisted-cache type everywhere."
Assert-Contains $homeCpp '(?s)load_weather_cache\(.*?weatherCacheIsValid.*?LittleFS\.remove\("/\.weather"\)' "The space theme must reject and remove a corrupt weather cache."
Assert-Contains $weatherCpp '(?s)cacheSize\s*==\s*sizeof\(weatherData\).*?weatherCacheIsValid.*?LittleFS\.remove\("/\.weather"\)' "The weather app must reject and remove a corrupt weather cache."

Assert-Contains $halCpp '(?s)fileSize\s*==\s*sizeof\(app_settings_file_header\)\s*\+\s*sizeof\(app_settings_save\).*?readCurrentAppSettings' "Current LittleFS settings must pass the format and checksum validation before use."
Assert-Contains $halCpp 'union\s+app_settings_scratch_storage' "Settings migration buffers must share static storage instead of consuming the 3584-byte main-task stack."
Assert-Contains $halCpp 'static\s+app_settings_scratch_storage\s+app_settings_scratch' "The settings scratch buffer must use static storage."
Assert-NotContains $halCpp 'app_setting\s+(candidate|payload|loaded|legacy)\s*=\s*\{\}' "The 2064-byte current settings object must never be allocated on the startup stack."
Assert-Contains $halCpp '(?s)fileSize\s*==\s*sizeof\(app_settings_save\).*?app_settings_scratch\.current.*?sanitizeAppSettingStrings\(legacy\).*?app_settings_save\s*=\s*legacy.*?fillWeatherDefaultsIfMissing' "Raw current settings must use static scratch storage and be terminated before strlen/strcmp."
Assert-Contains $halCpp '(?s)fileSize\s*==\s*sizeof\(legacy_app_setting\).*?sanitizeAppSettingStrings\(legacy\).*?migrateLegacyAppSettings' "Legacy LittleFS settings must be terminated before migration."
Assert-Contains $halCpp 'APP_SETTINGS_MAGIC' "Current settings need a format marker so truncated files cannot be mistaken for an older layout."
Assert-Contains $halCpp 'settingsChecksum' "Current settings need an integrity checksum before they are used at boot."
Assert-Contains $halCpp '(?s)cfg\.bin\.tmp.*?LittleFS\.rename' "Settings must be written completely before replacing the active file."
Assert-Contains $halCpp '(?s)memset\(\&app_settings_save.*?parseAppSettings\(default_app_setting\)' "Resetting settings must clear stale credential bytes before parsing defaults."

Write-Host "Persistent cache safety checks passed."
