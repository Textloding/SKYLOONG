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
Assert-Contains $homeCpp 'lv_obj_set_pos\(lbl_time,\s*24,\s*20\)' "Theme 2 time label must move further left so HH:mm:ss is not too far right on device."
Assert-Contains $homeCpp 'lv_obj_set_size\(lbl_time,\s*260,\s*42\)' "Theme 2 time label must keep a clipped, single-line HH:mm:ss box."
Assert-Contains $homeCpp 'lv_obj_set_size\(box_upper_left,\s*196,\s*110\)' "Theme 3 time card must be wide enough for HH:mm:ss."
Assert-Contains $homeCpp 'lv_obj_set_pos\(box_upper_left,\s*0,\s*10\)' "Theme 3 time card must move left to reduce the leading gap."
Assert-Contains $homeCpp 'lv_obj_set_style_pad_all\(box_upper_left,\s*0,\s*0\)' "Theme 3 time card must not lose width to default padding."
Assert-Contains $homeCpp 'lv_obj_set_style_text_font\(lbl_time,\s*&lv_font_montserrat_32,\s*0\)' "Theme 3 time label must use an enabled font size that fits HH:mm:ss on device."
Assert-Contains $homeCpp 'lv_obj_set_width\(lbl_time,\s*190\)' "Theme 3 time label must have a fixed width to avoid wrapping."
Assert-Contains $web 'theme-layout-20260709d' "Theme preview cache key must change when preview assets change."

foreach ($name in @("theme1.png", "theme2.png", "theme3.png", "theme4.png")) {
    Assert-FileExists (Join-Path $repoRoot "web\$name") "Embedded web preview $name is missing."
    Assert-FileExists (Join-Path $repoRoot "web_new\$name") "Local web preview $name is missing."
}

Write-Host "Theme layout checks passed."
