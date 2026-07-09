$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$homeCpp = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\apps\home.cpp")
$web = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "web_new\index.js")

function Assert-Contains($Text, $Pattern, $Message) {
    if ($Text -notmatch $Pattern) {
        throw $Message
    }
}

function Assert-FileExists($Path, $Message) {
    if (!(Test-Path $Path)) {
        throw $Message
    }
}

Assert-Contains $homeCpp 'lv_label_set_long_mode\(lbl_time,\s*LV_LABEL_LONG_CLIP\)' "Theme time labels must clip instead of wrapping seconds to a second line."
Assert-Contains $homeCpp 'lv_obj_set_pos\(lbl_time,\s*88,\s*20\)' "Theme 2 time label must start earlier to fit HH:mm:ss on device."
Assert-Contains $homeCpp 'lv_obj_set_size\(lbl_time,\s*232,\s*42\)' "Theme 2 time label must be wide enough for HH:mm:ss."
Assert-Contains $homeCpp 'lv_obj_set_pos\(lbl_time,\s*80,\s*30\)' "Theme 3 time label must start earlier to fit HH:mm:ss on device."
Assert-Contains $homeCpp 'lv_obj_set_size\(lbl_time,\s*240,\s*42\)' "Theme 3 time label must be wide enough for HH:mm:ss."
Assert-Contains $web 'theme-layout-20260709c' "Theme preview cache key must change when preview assets change."

foreach ($name in @("theme1.png", "theme2.png", "theme3.png", "theme4.png")) {
    Assert-FileExists (Join-Path $repoRoot "web\$name") "Embedded web preview $name is missing."
    Assert-FileExists (Join-Path $repoRoot "web_new\$name") "Local web preview $name is missing."
}

Write-Host "Theme layout checks passed."
