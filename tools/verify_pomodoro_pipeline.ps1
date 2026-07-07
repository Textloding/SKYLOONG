$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot

function Read-RepoFile([string]$relativePath) {
    Get-Content -Raw -Path (Join-Path $repoRoot $relativePath)
}

function Assert-Contains([string]$source, [string]$pattern, [string]$message) {
    if ($source -notmatch $pattern) {
        Write-Error $message
    }
}

$halHeader = Read-RepoFile "main\include\hal.h"
$appHeader = Read-RepoFile "main\include\APP_Def.hpp"
$appManager = Read-RepoFile "main\AppManagerLite.cpp"
$managerHeader = Read-RepoFile "main\include\AppManagerLite.h"
$protocol = Read-RepoFile "main\protocol.cpp"
$mainCpp = Read-RepoFile "main\main.cpp"
$halCpp = Read-RepoFile "main\hal.cpp"
$pomodoroCpp = Read-RepoFile "main\apps\Pomodoro.cpp"
$web = Read-RepoFile "web_new\index.js"
$mock = Read-RepoFile "web_new\mock_server.py"

Assert-Contains $appHeader 'class\s+AppPomodoro\s*:\s*public\s+BaseApp' "AppPomodoro class must be declared."
Assert-Contains $appHeader 'appid\s*=\s*7' "AppPomodoro must use appid 7."
Assert-Contains $appManager 'return\s+appPomodoro' "AppManagerLite must resolve appid 7 to AppPomodoro."
Assert-Contains $managerHeader 'pomodoro_enable' "App switch chain must gate Pomodoro with hal.pomodoro_enable."
Assert-Contains $managerHeader 'appPomodoro' "AppManagerLite must store the Pomodoro app pointer."
Assert-Contains $mainCpp 'AppPomodoro\s+appPomodoro' "main.cpp must instantiate AppPomodoro."
Assert-Contains $mainCpp 'pomodoro_init\s*\(\)' "main.cpp must start the background Pomodoro timer."
Assert-Contains $pomodoroCpp 'xTaskCreate' "Pomodoro must have a background task so countdown continues outside the page."
Assert-Contains $halHeader 'pomodoro_enable' "HAL must persist a Pomodoro enable flag."
Assert-Contains $halHeader 'pomodoroWaitingForConfirm' "HAL must expose Pomodoro confirmation state."
Assert-Contains $halHeader 'confirmPomodoro' "HAL must expose Pomodoro confirmation handling."
Assert-Contains $protocol 'pomodoroWaitingForConfirm\s*\(\)' "Protocol must check Pomodoro confirmation before switching apps."
Assert-Contains $protocol 'EVENT_POMODORO_CONFIRM' "Protocol must send Pomodoro confirmation instead of switching apps while ringing."
Assert-Contains $halCpp 'config_app_pomodoro' "Web server must provide /config_app_pomodoro."
Assert-Contains $halCpp 'pomodoro_enable' "/info must expose pomodoro_enable."
Assert-Contains $halCpp 'pomodoro_focus_min' "/info must expose Pomodoro durations."
Assert-Contains $halCpp 'pomodoro_tone' "/info must expose Pomodoro tone choice."
Assert-Contains $halCpp 'preview_pomodoro_tone' "Web server must provide a Pomodoro tone preview endpoint."
Assert-Contains $web 'appToggle\("pomodoro"' "Web overview must include a Pomodoro app toggle."
Assert-Contains $web 'fn\+~' "Web UI must tell users to press fn+~ to confirm the next Pomodoro."
Assert-Contains $web 'config_app_pomodoro' "Web UI must save Pomodoro settings."
Assert-Contains $web 'pomodoro-tone-grid' "Web UI must present Pomodoro tones as selectable cards instead of a bare select."
Assert-Contains $web 'data-play-pomodoro-tone' "Web UI must let users preview Pomodoro tones."
Assert-Contains $web 'data-upload-pomodoro-tone' "Web UI must let users upload a custom Pomodoro tone from the Pomodoro panel."
Assert-Contains $mock 'pomodoro_enable' "Mock server must include Pomodoro state."
Assert-Contains $mock 'config_app_pomodoro' "Mock server must handle Pomodoro settings."
Assert-Contains $mock 'preview_pomodoro_tone' "Mock server must handle Pomodoro tone preview requests."

Write-Host "Pomodoro pipeline source checks passed."
