$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$homeCpp = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\apps\home.cpp")
$web = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "web_new\index.js")

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

function Assert-FileExists($Path, $Message) {
    if (!(Test-Path $Path)) {
        throw $Message
    }
}

Assert-Contains $homeCpp 'lv_label_set_long_mode\(lbl_time,\s*LV_LABEL_LONG_CLIP\)' "Theme time labels must clip instead of wrapping seconds to a second line."
Assert-Contains $homeCpp '(?s)namespace theme_spartan.*?lv_obj_set_pos\(lbl_time,\s*56,\s*20\).*?lv_obj_set_size\(lbl_time,\s*246,\s*42\)' "Theme 2 on-device time label must move back right after the previous over-left adjustment."
Assert-Contains $homeCpp '(?s)namespace theme_fox.*?lv_obj_set_pos\(lbl_time,\s*36,\s*30\).*?lv_obj_set_size\(lbl_time,\s*246,\s*42\)' "Theme 3 on-device time label must move left in theme_fox, not in the default theme."
Assert-Contains $homeCpp '(?s)namespace theme_fox.*?lv_obj_set_pos\(img_am,\s*86\s*\+\s*8,\s*50\)' "Theme 3 AM/PM marker must follow the moved time label."
Assert-NotContains $homeCpp '(?s)namespace theme_space.*?lv_img_set_zoom\(img_astronaut' "Space theme must not use runtime LVGL image zoom; it can allocate transform buffers and reset on low memory."
Assert-Contains $homeCpp '(?s)namespace theme_space.*?void reset\(\).*?lv_img_cache_invalidate_src\(&img_space_bg\)' "Space theme must clear static LVGL pointers and invalidate cached image state on teardown."
Assert-Contains $homeCpp '(?s)namespace theme_space.*?lv_obj_add_event_cb\(star_layer,\s*draw_twinkling_stars,\s*LV_EVENT_DRAW_MAIN' "Space-theme stars must share one lightweight draw layer."
Assert-Contains $homeCpp '(?s)namespace theme_space.*?lv_obj_invalidate_area\(star_layer' "Space-theme twinkle animation must invalidate only the small star regions."
Assert-NotContains $homeCpp '(?s)namespace theme_space.*?lv_canvas_create' "Space-theme twinkle animation must not allocate a full-screen canvas."
Assert-Contains $homeCpp '(?s)namespace theme_space.*?uint8_t twinkle_wave\(\)' "All space-theme stars must share one breathing phase."
Assert-NotContains $homeCpp 'twinkle_frame\s*\+\s*star\.phase' "Space-theme stars must breathe together instead of using staggered phases."
Assert-Contains $homeCpp '(?s)void AppHome::destroy\(\).*?theme_space::reset\(\)' "Home app destroy must reset space-theme LVGL object pointers before the old screen is deleted."
Assert-NotContains $homeCpp '(?s)if \(current_theme != hal\.config_theme\).*?current_theme = hal\.config_theme;.*?return;' "Theme refresh must preserve the old current_theme until destroy() tears down the old theme."
Assert-Contains $web 'theme-layout-20260709d' "Theme preview cache key must change when preview assets change."

foreach ($name in @("theme1.png", "theme2.png", "theme3.png", "theme4.png")) {
    Assert-FileExists (Join-Path $repoRoot "web\$name") "Embedded web preview $name is missing."
    Assert-FileExists (Join-Path $repoRoot "web_new\$name") "Local web preview $name is missing."
}

Write-Host "Theme layout checks passed."
