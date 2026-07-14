$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$halCpp = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\hal.cpp")
$halH = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\include\hal.h")
$weatherCpp = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\apps\Weather.cpp")
$homeCpp = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\apps\home.cpp")
$web = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "web_new\index.js")
$readme = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "README.md")
$sdkconfig = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "sdkconfig")

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
    "weather_appkey_aliyun_72158",
    "weather_appkey_aliyun_10812",
    "weather_appkey_aliyun_50139",
    "weather_appkey_aliyun_71988",
    "weather_appsecret_aliyun_72158",
    "weather_appsecret_aliyun_10812",
    "weather_appsecret_aliyun_50139",
    "weather_appsecret_aliyun_71988"
)) {
    Assert-Contains $halH $field "App settings must persist Aliyun credential field $field."
    Assert-Contains $halCpp $field "HAL serialization must preserve Aliyun credential field $field."
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
Assert-Contains $weatherCpp 'X-Ca-Key' "Aliyun weather requests must support AppKey/AppSecret digest auth."
Assert-Contains $weatherCpp 'X-Ca-Signature' "Aliyun weather requests must send the digest signature header when AppKey/AppSecret are configured."
Assert-Contains $weatherCpp 'mbedtls_md_hmac' "Aliyun digest auth must compute HMAC signatures on-device."
Assert-Contains $weatherCpp 'canonicalPathAndParameters' "Aliyun digest auth must sign the sorted path and query string."
Assert-Contains $halCpp 'if \(slot == NULL \|\| key == NULL\)[\s\S]{0,40}return;' "HAL must accept an empty AppCode/API key to clear stale provider credentials."
Assert-Contains $halCpp 'if \(slot == NULL \|\| appKey == NULL\)[\s\S]{0,40}return;' "HAL must accept an empty Aliyun AppKey to clear stale provider credentials."
Assert-Contains $halCpp 'if \(slot == NULL \|\| appSecret == NULL\)[\s\S]{0,40}return;' "HAL must accept an empty Aliyun AppSecret to clear stale provider credentials."
Assert-Contains $halCpp 'if \(isAliyunWeatherProviderName\(provider\) && appCode != NULL\)\s*key = appCode;' "Aliyun weather tests must respect an explicitly empty AppCode instead of falling back to a saved one."
Assert-Contains $web 'weatherProviderKeys' "Web UI must track configured keys per weather source."
Assert-Contains $web 'weatherProviderKeyValues' "Web UI must receive and show saved weather keys per source."
Assert-Contains $web 'weatherKeyDrafts' "Web UI must keep user-entered keys while switching sources."
Assert-Contains $web 'weatherCredentialDrafts' "Web UI must keep Aliyun AppCode/AppKey/AppSecret drafts while switching sources."
Assert-Contains $web 'weather_provider_credentials' "Web UI must save all Aliyun credential fields per source."
Assert-Contains $web 'data-weather-credential="appcode"' "Web UI must expose Aliyun AppCode separately."
Assert-Contains $web 'data-weather-credential="appkey"' "Web UI must expose Aliyun AppKey separately."
Assert-Contains $web 'data-weather-credential="appsecret"' "Web UI must expose Aliyun AppSecret separately."
Assert-Contains $web 'hasOwn\(cfg,\s*"weather_appcode"\)' "Web UI normalization must preserve explicit empty Aliyun AppCode values so stale credentials can be cleared."
Assert-Contains $web 'weatherProviderEndpoints' "Web UI must track saved endpoint per weather source."
Assert-Contains $web 'weatherEndpointDrafts' "Web UI must keep endpoint drafts while switching sources."
Assert-Contains $web 'isWeatherEditing' "Web UI polling must not overwrite active weather edits."
Assert-Contains $web 'defaultWeatherProviderEndpointFor\(provider\)' "Changing weather source must switch the endpoint input to that source default or saved endpoint."
Assert-Contains $web 'if \(key === "weather_provider"\) return;' "Weather source select must not be handled by generic data-cfg input before the dedicated change handler."
Assert-Contains $web '(?s)<section class="panel span-two">.{0,500}<div class="weather-layout">' "Weather panel must span two dashboard columns so settings and docs are not squeezed."
Assert-Contains $web '\.\.\.state\.weatherEndpointDrafts' "Saving weather config must include endpoint drafts for every provider."
Assert-Contains $web '\.\.\.state\.weatherKeyDrafts' "Saving weather config must include key drafts for every provider."
Assert-Contains $web 'weather_provider_key_values:\s*nextKeyValues' "Switching weather source must carry the whole per-provider key map."
Assert-Contains $web '/weather_test' "Management UI must call the firmware weather test endpoint."
Assert-Contains $halCpp 'server\.on\("/weather_test"' "Firmware must expose a weather test endpoint."
Assert-Contains $weatherCpp 'testWeatherProvider' "Firmware weather app must expose a reusable provider test function."
Assert-Contains $halH 'weather_publickey_seniverse' "Seniverse public key needs its own persistent field."
Assert-Contains $halCpp 'weather_publickey_seniverse' "HAL must save and return the Seniverse public key."
Assert-Contains $halCpp 'settimeofday\s*\(' "NTP sync must update the system UTC epoch for signed weather APIs."
Assert-NotContains $sdkconfig 'CONFIG_NEWLIB_TIME_SYSCALL_USE_NONE=y' "System time cannot be disabled because Seniverse signing needs a Unix timestamp."
Assert-Contains $sdkconfig 'CONFIG_NEWLIB_TIME_SYSCALL_USE_HRT=y' "System time must use the ESP high-resolution timer."
Assert-Contains $weatherCpp 'time\s*\(NULL\)[\s\S]*?hal\.NTPSync\s*\(\)[\s\S]*?time\s*\(NULL\)' "Seniverse signing must retry after NTP sync when the system epoch is invalid."
Assert-Contains $halCpp 'if \(isAliyunWeatherProviderName\(provider\) && appCode != NULL\)' "An empty Aliyun AppCode field must not erase another provider key during testing."
Assert-Contains $halCpp 'static app_settings_scratch_storage app_settings_scratch;' "Large app-setting migration buffers must not live on the main task stack."
Assert-Contains $web 'data-weather-seniverse="publickey"' "Management UI must expose the Seniverse public key."
Assert-Contains $web 'data-weather-seniverse="privatekey"' "Management UI must expose the Seniverse private key."
Assert-Contains $weatherCpp 'HMAC-SHA1' "Seniverse public/private authentication must use the documented HMAC-SHA1 signature."
Assert-Contains $weatherCpp 'seniverseLocation\(\)' "Seniverse requests must share location normalization."
Assert-Contains $weatherCpp 'weatherLat \+ ":" \+ weatherLon' "Seniverse must prefer coordinates because Chinese city names are not accepted directly."
Assert-Contains $weatherCpp 'days=3' "Seniverse free users must request no more than three forecast days."
Assert-Contains $weatherCpp 'static bool saveWeatherCache\(\)' "Weather updates and tests must share one cache writer for themes."
Assert-Contains $weatherCpp 'jsonObj\(jsonObj\(body, "f1"\), "index"\)' "Aliyun 10812 life suggestions must be read from f1.index."
Assert-Contains $weatherCpp 'jsonObj\(index, "clothes"\).*"title"' "Aliyun 10812 clothing suggestion must use clothes.title."
Assert-Contains $weatherCpp 'jsonObj\(index, "sports"\).*"title"' "Aliyun 10812 sports suggestion must use sports.title."
Assert-Contains $weatherCpp 'jsonObj\(index, "cold"\).*"title"' "Aliyun 10812 cold suggestion must use cold.title."
Assert-Contains $weatherCpp 'jsonObj\(index, "wash_car"\).*"title"' "Aliyun 10812 car-washing suggestion must use wash_car.title."
Assert-Contains $weatherCpp 'if \(ok\)[\s\S]{0,120}saveWeatherCache\(\)' "A successful weather test must immediately publish data to the shared theme cache."
Assert-NotContains $weatherCpp 'weather_update_cnt\s*%\s*5' "Successful weather updates must not wait five cycles before themes receive data."
Assert-Contains $halCpp '"webserver",\s*12288' "Webserver task needs enough stack for TLS weather tests."
Assert-NotContains $weatherCpp 'LittleFS\.open\("\.weather"' "LittleFS weather cache paths must start with /."
Assert-Contains $weatherCpp 'LittleFS\.open\("/\.weather"' "Weather cache must use an absolute LittleFS path."
Assert-NotContains $homeCpp 'LittleFS\.open\("\.weather"' "Home weather cache paths must start with /."
Assert-Contains $homeCpp 'LittleFS\.open\("/\.weather"' "Home weather cache must use an absolute LittleFS path."
Assert-NotContains $weatherCpp 'LittleFS\.open\("/\.weather",\s*"a"\)' "Weather cache reads must not mutate LittleFS before validation."
Assert-NotContains $homeCpp 'LittleFS\.open\("/\.weather",\s*"a"\)' "Home weather cache reads must not mutate LittleFS before validation."
Assert-Contains $weatherCpp '(?s)weatherCacheIsValid.*?LittleFS\.remove\("/\.weather"\)' "Weather app must discard a corrupt persisted cache."
Assert-Contains $homeCpp '(?s)weatherCacheIsValid.*?LittleFS\.remove\("/\.weather"\)' "Space theme must discard a corrupt persisted cache."
Assert-Contains $web 'weather-layout' "Weather settings and tutorial must support side-by-side layout."
Assert-NotContains $web 'data-cfg="weather" type="password"' "Weather API Key/AppCode input must be visible text, not a password box."
Assert-NotContains $web 'type="password" data-cfg="weather"' "Weather API Key/AppCode input must be visible text, not a password box."
Assert-NotContains $web 'hasSavedKey && !keyDraft' "Weather API Key/AppCode must not use a hidden-saved placeholder anymore."
Assert-Contains $web 'type="text" autocomplete="off" value' "Management UI must show the saved weather key as editable text."
Assert-Contains $halCpp "weather_provider_key_values" "HAL JSON must return saved weather key values because the management UI needs to show them."
Assert-Contains $halCpp "weather_key_seniverse" "HAL JSON must return saved Seniverse key."
Assert-Contains $halCpp "jsonbuffer\[16384\]" "HAL JSON buffer must have enough room for per-source endpoints, visible keys, and Aliyun credentials."
Assert-Contains $web "weather-docs" "Management UI must include in-console weather source tutorial docs."
Assert-Contains $web "Host \+ Path" "Management UI must tell Aliyun users to copy Host + Path from API debug/interface docs."
Assert-Contains $readme "AppCode" "README must explain how Aliyun AppCode is used."
Assert-Contains $readme "AppKey" "README must explain how Aliyun AppKey is used."
Assert-Contains $readme "AppSecret" "README must explain how Aliyun AppSecret is used."
Assert-Contains $readme "Host \+ Path" "README must tell users where to find Aliyun API debug/interface details."
Assert-Contains $readme "cmapi00072158" "README must list Aliyun market product links."
Assert-Contains $readme "weather01\.market\.alicloudapi\.com" "README must explain weather endpoint addresses."

Write-Host "Weather source checks passed."
