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

function Assert-FileExists([string]$relativePath, [string]$message) {
    if (-not (Test-Path (Join-Path $repoRoot $relativePath))) {
        Write-Error $message
    }
}

$halCpp = Read-RepoFile "main\hal.cpp"
$i18n = Read-RepoFile "main\internationalization.cpp"
$keytone1 = Read-RepoFile "main\keytone\keytone1.h"
$keytone2 = Read-RepoFile "main\keytone\keytone2.h"
$keytone3 = Read-RepoFile "main\keytone\keytone3.h"
$keytone4 = Read-RepoFile "main\keytone\keytone4.h"
$keytone5 = Read-RepoFile "main\keytone\keytone5.h"
$keytone6 = Read-RepoFile "main\keytone\keytone6.h"
$i2sHeader = Read-RepoFile "components\arduino\libraries\ESP_I2S\src\ESP_I2S.h"
$i2sCpp = Read-RepoFile "components\arduino\libraries\ESP_I2S\src\ESP_I2S.cpp"
$webNew = Read-RepoFile "web_new\index.js"
$web = Read-RepoFile "web\index.js"

Assert-FileExists "main\keytone\keytone5.h" "Built-in keytone 5 header must exist."
Assert-FileExists "main\keytone\keytone6.h" "Built-in keytone 6 header must exist."
Assert-Contains $halCpp '#include\s+"keytone/keytone4\.h"' "HAL must include the extra built-in keytone 4 asset."
Assert-Contains $halCpp '#include\s+"keytone/keytone5\.h"' "HAL must include the extra built-in keytone 5 asset."
Assert-Contains $halCpp '#include\s+"keytone/keytone6\.h"' "HAL must include the extra built-in keytone 6 asset."
Assert-Contains $halCpp 'config_keytone\s*==\s*5' "HAL must play built-in keytone option 5."
Assert-Contains $halCpp 'config_keytone\s*==\s*6' "HAL must play built-in keytone option 6."
Assert-Contains $halCpp 'config_keytone\s*==\s*7' "HAL must play built-in keytone option 7."
Assert-Contains $keytone1 'const\s+unsigned\s+char\s+__keytone_keytone1_wav\[\]' "Built-in keytone 1 must stay in flash instead of writable DRAM."
Assert-Contains $keytone2 'const\s+unsigned\s+char\s+__keytone_keytone2_wav\[\]' "Built-in keytone 2 must stay in flash instead of writable DRAM."
Assert-Contains $keytone3 'const\s+unsigned\s+char\s+__keytone_keytone3_wav\[\]' "Built-in keytone 3 must stay in flash instead of writable DRAM."
Assert-Contains $keytone4 'const\s+unsigned\s+char\s+__keytone_keytone4_wav\[\]' "Extra keytone 4 must be const so it stays out of DRAM."
Assert-Contains $keytone5 'const\s+unsigned\s+char\s+__keytone_keytone5_wav\[\]' "Extra keytone 5 must be const so it stays out of DRAM."
Assert-Contains $keytone6 'const\s+unsigned\s+char\s+__keytone_keytone6_wav\[\]' "Extra keytone 6 must be const so it stays out of DRAM."
Assert-Contains $i2sHeader 'bool\s+playWAV\(const\s+uint8_t\s+\*data,\s*size_t\s+len\)' "I2S playWAV must accept const WAV buffers and report playback failure."
Assert-Contains $i2sHeader 'size_t\s+write\(const\s+uint8_t\s+\*buffer,\s*size_t\s+size\)' "I2S write must accept const buffers for flash-resident audio."
Assert-Contains $i2sCpp 'bool\s+I2SClass::playWAV\(const\s+uint8_t\s+\*data,\s*size_t\s+len\)' "I2S playWAV implementation must accept const WAV buffers and report playback failure."
Assert-Contains $i2sCpp 'size_t\s+I2SClass::write\(const\s+uint8_t\s+\*buffer,\s*size_t\s+size\)' "I2S write implementation must accept const buffers for flash-resident audio."
Assert-Contains $i18n '\u5F39\u8DF3\u4E09\u8FDE' "On-device settings must expose the bounce keytone label."
Assert-Contains $i18n '\u7BEE\u7403\u5F8B\u52A8' "On-device settings must expose the basketball rhythm keytone label."
Assert-Contains $i18n '\u8DA3\u5473\u4E0A\u626C' "On-device settings must expose the rising fun keytone label."
Assert-Contains $i18n 'Bounce Triple' "On-device English settings must expose the bounce keytone label."
Assert-Contains $i18n 'Basketball Rhythm' "On-device English settings must expose the basketball rhythm keytone label."
Assert-Contains $i18n 'Fun Rise' "On-device English settings must expose the rising fun keytone label."
Assert-Contains $webNew 'toneButton\(3,' "Editable web UI must expose built-in keytone 3."
Assert-Contains $webNew 'toneButton\(5,' "Editable web UI must expose built-in keytone 5."
Assert-Contains $webNew 'toneButton\(6,' "Editable web UI must expose built-in keytone 6."
Assert-Contains $webNew 'toneButton\(7,' "Editable web UI must expose built-in keytone 7."
Assert-Contains $web 'toneButton\(3,' "Embedded web UI must expose built-in keytone 3."
Assert-Contains $web 'toneButton\(5,' "Embedded web UI must expose built-in keytone 5."
Assert-Contains $web 'toneButton\(6,' "Embedded web UI must expose built-in keytone 6."
Assert-Contains $web 'toneButton\(7,' "Embedded web UI must expose built-in keytone 7."

Write-Host "Keytone pipeline source checks passed."
