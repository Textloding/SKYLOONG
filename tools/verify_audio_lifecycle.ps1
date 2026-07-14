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
$decoder = Read-RepoFile "main\mpegDecoder.cpp"
$i2sHeader = Read-RepoFile "components\arduino\libraries\ESP_I2S\src\ESP_I2S.h"
$i2sSource = Read-RepoFile "components\arduino\libraries\ESP_I2S\src\ESP_I2S.cpp"
$audioBegin = [regex]::Match($halSource, '(?s)bool HAL::audio_begin_playback\(\).*?(?=void HAL::audio_end_playback\(\))').Value
$audioInit = [regex]::Match($halSource, '(?s)esp_err_t HAL::audio_init\(\).*?(?=void HAL::audio_stop\(\))').Value
$audioStop = [regex]::Match($halSource, '(?s)void HAL::audio_stop\(\).*?(?=bool HAL::audio_begin_playback\(\))').Value
$audioEnd = [regex]::Match($halSource, '(?s)void HAL::audio_end_playback\(\).*?(?=void HAL::audio_shutdown\(\))').Value
$audioShutdown = [regex]::Match($halSource, '(?s)void HAL::audio_shutdown\(\).*?(?=void HAL::init\(\))').Value
$setVolume = [regex]::Match($halSource, '(?s)void HAL::setVolume\(.*?(?=void HAL::LOCKLV\(\))').Value
$playWav = [regex]::Match($i2sSource, '(?s)void I2SClass::playWAV\(.*?(?=bool I2SClass::playMP3\()').Value
$playMp3 = [regex]::Match($i2sSource, '(?s)bool I2SClass::playMP3\(.*?(?=void I2SClass::stop\()').Value

Assert-Contains $halHeader '_audio_mutex' "HAL must expose a dedicated recursive audio mutex."
Assert-Contains $halHeader '_audio_owner' "HAL must track the task that owns the audio lifecycle lock."
Assert-Contains $halHeader '_audio_shutdown_requested' "HAL must publish shutdown intent before waiting for playback ownership."
Assert-Contains $halHeader 'audio_begin_playback\(\)' "HAL must keep a complete playback ownership API."
Assert-Contains $halSource 'xSemaphoreCreateRecursiveMutex' "Audio lifecycle synchronization must use a recursive FreeRTOS mutex."
Assert-Contains $halSource 'audio_lock\(' "Audio lifecycle methods must acquire the dedicated audio mutex."
Assert-Contains $halSource 'audio_unlock\(' "Audio lifecycle methods must release the dedicated audio mutex."
Assert-Contains $audioInit 'audio_lock\(' "audio_init must serialize codec and I2S initialization."
Assert-Contains $audioStop 'audio_lock\(' "audio_stop must serialize I2S cancellation."
Assert-Contains $audioShutdown 'audio_lock\(' "audio_shutdown must serialize codec and I2S teardown."
Assert-Contains $setVolume 'audio_lock\(' "setVolume must serialize ES8311 register writes."
Assert-Contains $halSource 'AUDIO_DMA_TAIL_GUARD_MS\s*=\s*50' "Playback shutdown must define a 50 ms DMA tail guard."
Assert-Contains $audioEnd '(?s)delay\(AUDIO_DMA_TAIL_GUARD_MS\).*?digitalWrite\(AUDIO_AMP_CTRL,\s*LOW\)' "Playback shutdown must wait for the final DMA samples before muting the amplifier."
Assert-Contains $halSource '_audio_owner\s*=\s*current' "Playback must retain ownership until audio_end_playback."
Assert-Contains $halSource '_audio_owner\s*=\s*NULL' "Playback ownership must be released after the DMA tail drains."
Assert-Contains $halSource 'audio_stop' "Audio stop must participate in lifecycle synchronization."
Assert-Contains $decoder 'audio_begin_playback\(\)' "MPEG playback must acquire audio ownership before configuring/writing I2S."
Assert-Contains $decoder '(?s)audio_begin_playback\(\).*?i2s\.configureTX' "MPEG I2S configuration must occur inside the playback ownership scope."
Assert-Contains $decoder 'audio_end_playback\(\)' "MPEG playback must release audio ownership after the final frame."
Assert-Contains $i2sHeader 'void\s+clearStop\(\)' "I2S must expose an explicit way to clear an earlier asynchronous stop request."
Assert-Contains $i2sSource '(?s)void\s+I2SClass::clearStop\(\).*?_stop\s*=\s*false' "I2S clearStop must restore writes after a cancelled playback."
Assert-Contains $audioBegin 'i2s\.clearStop\(\)' "Every new playback scope must clear a stale I2S stop request."
Assert-Contains $audioBegin '(?s)_audio_shutdown_requested.*?audio_unlock\(\).*?return false' "New playback must be rejected while audio shutdown is pending."
Assert-NotContains $playWav '_stop\s*=\s*false' "WAV playback must not erase a stop request issued after playback ownership was acquired."
Assert-NotContains $playMp3 '_stop\s*=\s*false' "MP3 playback must not erase a stop request issued after playback ownership was acquired."
Assert-Contains $audioShutdown '(?s)i2s\.stop\(\).*?gif_vid_stop\s*=\s*true.*?audio_lock\(' "Audio shutdown must cancel looping media before waiting for playback ownership."
Assert-Contains $audioShutdown '(?s)_audio_shutdown_requested(?:\s*=\s*true|\.store\(true\)).*?i2s\.stop\(\)' "Audio shutdown intent must be visible before issuing asynchronous cancellation."
Assert-Contains $audioShutdown 'pdMS_TO_TICKS\(' "Audio shutdown must use a finite lock wait after requesting cancellation."

Write-Host "Audio lifecycle source checks passed."
