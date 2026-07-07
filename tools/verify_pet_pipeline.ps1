$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot

function Read-RepoFile([string]$relativePath) {
    Get-Content -Raw -Encoding UTF8 -Path (Join-Path $repoRoot $relativePath)
}

function Assert-Contains([string]$source, [string]$pattern, [string]$message) {
    if ($source -notmatch $pattern) {
        Write-Error $message
    }
}

function Assert-NotContains([string]$source, [string]$pattern, [string]$message) {
    if ($source -match $pattern) {
        Write-Error $message
    }
}

function Assert-FileExists([string]$relativePath, [string]$message) {
    if (-not (Test-Path (Join-Path $repoRoot $relativePath))) {
        Write-Error $message
    }
}

$appHeader = Read-RepoFile "main\include\APP_Def.hpp"
$managerHeader = Read-RepoFile "main\include\AppManagerLite.h"
$appManager = Read-RepoFile "main\AppManagerLite.cpp"
$mainCpp = Read-RepoFile "main\main.cpp"
$halHeader = Read-RepoFile "main\include\hal.h"
$halCpp = Read-RepoFile "main\hal.cpp"
$petCpp = if (Test-Path (Join-Path $repoRoot "main\apps\Pet.cpp")) { Read-RepoFile "main\apps\Pet.cpp" } else { "" }
$webNew = Read-RepoFile "web_new\index.js"
$web = Read-RepoFile "web\index.js"
$webNewCss = Read-RepoFile "web_new\index.css"
$webCss = Read-RepoFile "web\index.css"
$mock = Read-RepoFile "web_new\mock_server.py"
$updateWeb = Read-RepoFile "updateWeb.sh"
$petAssetDoc = if (Test-Path (Join-Path $repoRoot "docs\pet-assets.md")) { Read-RepoFile "docs\pet-assets.md" } else { "" }
$petTitleText = [regex]::Escape(([char]0x952E).ToString() + ([char]0x76D8).ToString() + ([char]0x5BA0).ToString() + ([char]0x7269).ToString())

Assert-FileExists "main\apps\Pet.cpp" "Keyboard pet app source must exist."
Assert-FileExists "assets\pet\dog_medium.png" "Keyboard pet must keep the upstream CC0 dog sprite sheet in assets/pet."
Assert-FileExists "main\pet_dog_sprites.c" "Keyboard pet must compile the dog sprite sheet into LVGL image frames."
Assert-FileExists "docs\pet-assets.md" "Keyboard pet must document open-source pet asset attribution."
Assert-Contains $appHeader 'class\s+AppPet\s*:\s*public\s+BaseApp' "AppPet class must be declared."
Assert-Contains $appHeader 'appid\s*=\s*8' "AppPet must use appid 8."
Assert-Contains $managerHeader 'appPet' "AppManagerLite must store the pet app pointer."
Assert-Contains $managerHeader 'pet_enable' "App switch chain must gate the pet with hal.pet_enable."
Assert-Contains $appManager 'return\s+appPet' "AppManagerLite must resolve appid 8 to AppPet."
Assert-Contains $mainCpp 'AppPet\s+appPet' "main.cpp must instantiate AppPet."
Assert-Contains $mainCpp 'appPet\.init\s*\(\)' "main.cpp must initialize AppPet."
Assert-Contains $mainCpp 'appManagerLite\.appPet\s*=\s*&appPet' "main.cpp must register AppPet with the manager."
Assert-Contains $halHeader 'pet_enable' "HAL must persist the pet enable flag."
Assert-Contains $halHeader 'pet_theme' "HAL must persist the pet theme."
Assert-Contains $halHeader 'pet_reactivity' "HAL must persist the pet interaction reactivity."
Assert-Contains $halHeader 'pet_name' "HAL must persist the pet display name."
Assert-Contains $halHeader 'pet_keypress_seq' "HAL must expose a keypress counter for pet interaction."
Assert-Contains $halCpp 'pet_enable\s*=\s*pref\.getBool\("pet_enable"' "HAL must load the pet enable flag."
Assert-Contains $halCpp 'pref\.getUInt\("pet_theme"' "HAL must load the pet theme."
Assert-Contains $halCpp 'pref\.getUInt\("pet_reactivity"' "HAL must load the pet reactivity."
Assert-Contains $halCpp 'pet_name' "HAL must handle the pet name."
Assert-Contains $halCpp 'cJSON_AddBoolToObject\(json,\s*"pet_enable"' "/info must expose pet_enable."
Assert-Contains $halCpp 'cJSON_AddNumberToObject\(json,\s*"pet_theme"' "/info must expose pet_theme."
Assert-Contains $halCpp 'cJSON_AddNumberToObject\(json,\s*"pet_reactivity"' "/info must expose pet_reactivity."
Assert-Contains $halCpp 'cJSON_AddStringToObject\(json,\s*"pet_name"' "/info must expose pet_name."
Assert-Contains $halCpp 'config_app_pet' "Web server must provide /config_app_pet."
Assert-Contains $halCpp 'pet_keypress_seq\+\+' "Keypress events must feed the pet interaction counter."
Assert-Contains $halCpp 'webserver/dog_medium\.h' "Web server must include the embedded dog sprite asset."
Assert-Contains $halCpp 'server\.on\("/dog_medium\.png"' "Web server must serve the dog sprite asset."
Assert-Contains $halCpp '__web_dog_medium_png_gz' "Web server must send the embedded dog sprite gzip payload."
Assert-Contains $petCpp 'AppPet::setup' "Pet app must implement setup."
Assert-Contains $petCpp 'AppPet::loop' "Pet app must implement loop."
Assert-Contains $petCpp 'AppPet::destroy' "Pet app must implement destroy."
Assert-Contains $petCpp 'pet_keypress_seq' "Pet app must react to keypress activity."
Assert-Contains $petCpp 'pet_dog_' "Pet app must render the open-source dog sprite frames."
Assert-Contains $petCpp 'lv_img_create' "Pet app must use LVGL image widgets for the open-source dog sprite."
Assert-Contains $petCpp 'lv_img_set_src' "Pet app must animate by swapping dog sprite frames."
Assert-Contains $petCpp 'petDogFrameForMood' "Pet app must map typing mood to dog animation frames."
Assert-Contains $petCpp 'petTheme' "Pet app must apply selectable visual themes."
Assert-Contains $petCpp 'pet_reactivity' "Pet app must apply configurable interaction reactivity."
Assert-Contains $petCpp 'pet_name' "Pet app must display the configured pet name."
Assert-Contains $petCpp 'lv_obj_set_style_bg_color' "Pet app should use LVGL object styling for compatible rendering."
Assert-NotContains $petCpp 'lv_canvas_get_draw_ctx' "Pet app must not use lv_canvas_get_draw_ctx because this LVGL version does not provide it."
Assert-NotContains $petCpp 'pet_body' "Pet app must not fall back to the old blocky robot body."
Assert-NotContains $petCpp 'pet_eye_left' "Pet app must not fall back to the old blocky robot eyes."
Assert-Contains $petCpp $petTitleText "Pet app must use a Chinese title."
Assert-Contains $webNew 'pet_enable' "Editable web UI must normalize pet_enable."
Assert-Contains $webNew 'pet_name' "Editable web UI must normalize pet_name."
Assert-Contains $webNew 'pet_theme' "Editable web UI must normalize pet_theme."
Assert-Contains $webNew 'pet_reactivity' "Editable web UI must normalize pet_reactivity."
Assert-Contains $webNew 'petPreview' "Editable web UI must show a pet preview."
Assert-Contains $webNew 'dog_medium.png' "Editable web UI must preview the same open-source dog sprite sheet."
Assert-Contains $webNew 'pet-sprite' "Editable web UI must render the dog sprite preview."
Assert-NotContains $webNew '--pet-sprite:url\("' "Editable web UI must not put a double-quoted sprite URL inside a double-quoted style attribute."
Assert-Contains $webNew 'data-save-pet' "Editable web UI must save pet customization."
Assert-Contains $webNew 'appToggle\("pet"' "Editable web overview must include a pet app toggle."
Assert-Contains $webNew $petTitleText "Editable web UI must name the pet in Chinese."
Assert-Contains $web 'appToggle\("pet"' "Embedded web UI must include a pet app toggle."
Assert-Contains $web 'petPreview' "Embedded web UI must show a pet preview."
Assert-Contains $web 'dog_medium.png' "Embedded web UI must preview the same open-source dog sprite sheet."
Assert-NotContains $web '--pet-sprite:url\("' "Embedded web UI must not put a double-quoted sprite URL inside a double-quoted style attribute."
Assert-Contains $webCss 'pet-sprite' "Embedded web CSS must style the dog sprite preview."
Assert-Contains $webNewCss 'pet-sprite' "Editable web CSS must style the dog sprite preview."
Assert-NotContains $webNew 'pet-robot' "Editable web UI must not render the old CSS robot preview."
Assert-NotContains $web 'pet-robot' "Embedded web UI must not render the old CSS robot preview."
Assert-Contains $updateWeb 'dog_medium\.png\.gz' "Web update script must bundle the dog sprite asset."
Assert-Contains $updateWeb 'dog_medium\.h' "Web update script must generate the dog sprite header."
Assert-Contains $mock 'pet_enable' "Mock server must include pet state."
Assert-Contains $mock 'pet_theme' "Mock server must include pet theme state."
Assert-Contains $mock 'pet_reactivity' "Mock server must include pet reactivity state."
Assert-Contains $mock 'pet_name' "Mock server must include pet name state."
Assert-Contains $mock '"pet"' "Mock server app toggle loop must include pet."
Assert-Contains $mock 'config_app_\{app\}' "Mock server must handle generic app toggles."
Assert-Contains $petAssetDoc 'OpenGameArt' "Pet asset documentation must mention the source site."
Assert-Contains $petAssetDoc 'https://opengameart.org/content/dog-3' "Pet asset documentation must link the source page."
Assert-Contains $petAssetDoc 'CC0' "Pet asset documentation must record the CC0 license."

Write-Host "Keyboard pet pipeline source checks passed."
