$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$halCpp = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\hal.cpp")
$halH = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\include\hal.h")
$weatherCpp = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\apps\Weather.cpp")
$web = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "web_new\index.js")
$readme = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "README.md")

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

Assert-NotContains $halCpp 'default_weather_provider\[\]\s*=\s*"openmeteo"' "Open-Meteo must not remain the default provider."
Assert-NotContains $web 'openmeteo\s*:' "Web weather source selector must not offer Open-Meteo."
Assert-NotContains $weatherCpp 'fallback to Open-Meteo' "Firmware weather fallback must not call Open-Meteo."

foreach ($provider in @("aliyun_72158", "aliyun_10812", "aliyun_50139", "aliyun_71988")) {
    Assert-Contains $halCpp $provider "Firmware must accept provider $provider."
    Assert-Contains $weatherCpp $provider "Weather fetcher must route provider $provider."
    Assert-Contains $web $provider "Web UI must expose provider $provider."
}

foreach ($field in @(
    "weather_key_seniverse",
    "weather_key_qweather",
    "weather_key_aliyun_72158",
    "weather_key_aliyun_10812",
    "weather_key_aliyun_50139",
    "weather_key_aliyun_71988"
)) {
    Assert-Contains $halH $field "App settings must persist per-provider key field $field."
    Assert-Contains $halCpp $field "HAL serialization must preserve $field."
}

foreach ($field in @(
    "weather_endpoint_seniverse",
    "weather_endpoint_qweather",
    "weather_endpoint_aliyun_72158",
    "weather_endpoint_aliyun_10812",
    "weather_endpoint_aliyun_50139",
    "weather_endpoint_aliyun_71988"
)) {
    Assert-Contains $halH $field "App settings must persist per-provider endpoint field $field."
    Assert-Contains $halCpp $field "HAL serialization must preserve $field."
}

Assert-Contains $weatherCpp 'String\s+auth\s*=\s*String\("APPCODE "\)\s*\+\s*appCode[\s\S]*?addHeader\("Authorization",\s*auth\)' "Aliyun weather requests must send Authorization: APPCODE."
Assert-Contains $web 'weatherProviderKeys' "Web UI must track configured keys per weather source."
Assert-Contains $web 'weatherKeyDrafts' "Web UI must keep user-entered keys while switching sources."
Assert-Contains $web 'weatherProviderEndpoints' "Web UI must track saved endpoint per weather source."
Assert-Contains $web 'weatherEndpointDrafts' "Web UI must keep endpoint drafts while switching sources."
Assert-Contains $web 'isWeatherEditing' "Web UI polling must not overwrite active weather edits."
Assert-Contains $readme 'AppCode' "README must explain how Aliyun AppCode is used."
Assert-Contains $readme 'Host \+ Path' "README must tell users where to find Aliyun API debug/interface details."
Assert-Contains $readme 'cmapi00072158' "README must list Aliyun market product links."
Assert-Contains $readme 'weather01\.market\.alicloudapi\.com' "README must explain weather endpoint addresses."

Write-Host "Weather source checks passed."
