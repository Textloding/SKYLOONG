$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$halCpp = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\hal.cpp")
$halH = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\include\hal.h")
$mainCpp = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\main.cpp")
$protocolCpp = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\protocol.cpp")
$mpegCpp = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "main\mpegDecoder.cpp")
$sdkconfig = Get-Content -Raw -Encoding UTF8 (Join-Path $repoRoot "sdkconfig")
$audioInit = [regex]::Match($halCpp, '(?s)esp_err_t HAL::audio_init\(\).*?(?=void HAL::audio_stop\()').Value
$setVolume = [regex]::Match($halCpp, '(?s)void HAL::setVolume\(.*?(?=void HAL::LOCKLV\()').Value
$audioPlayback = [regex]::Match($halCpp, '(?s)bool HAL::audio_begin_playback\(\).*?(?=void HAL::audio_end_playback\()').Value
$audioPlaybackEnd = [regex]::Match($halCpp, '(?s)void HAL::audio_end_playback\(\).*?(?=void HAL::audio_shutdown\()').Value

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

Assert-Contains $halH 'bool\s+audio_ready\s*=\s*false' "HAL must track whether the codec and I2S output are ready."
Assert-NotContains $halCpp 'ESP_ERROR_CHECK\s*\(\s*es8311_init' "ES8311 initialization failure must not abort and reboot the device."
Assert-Contains $audioInit '(?s)digitalWrite\(AUDIO_AMP_CTRL,\s*LOW\).*?es8311_init.*?i2s\.begin.*?audio_ready\s*=\s*true' "The amplifier must stay muted until codec and I2S initialization complete."
Assert-NotContains $audioInit 'digitalWrite\(AUDIO_AMP_CTRL,\s*HIGH\)' "audio_init must not enable the amplifier before the UI or media path restores volume."
Assert-Contains $setVolume '(?s)audio_ready.*?es8311_voice_volume_set' "Volume writes must be skipped while the codec is unavailable."
Assert-NotContains $setVolume 'digitalWrite\(AUDIO_AMP_CTRL,\s*HIGH\)' "Changing volume or completing the first display flush must not leave the idle amplifier enabled."
Assert-Contains $audioPlayback '(?s)setVolume\(.*?digitalWrite\(AUDIO_AMP_CTRL,\s*HIGH\)' "The amplifier may be enabled only after playback is requested and codec volume is ready."
Assert-Contains $audioPlaybackEnd 'digitalWrite\(AUDIO_AMP_CTRL,\s*LOW\)' "Every completed playback must return the amplifier to its idle-low state."
Assert-Contains $halCpp '(?s)playWavWithAmplifier.*?audio_begin_playback.*?i2s\.playWAV.*?audio_end_playback' "WAV playback must gate the amplifier for only the playback duration."
Assert-Contains $halCpp '(?s)playAudioFileFromLittleFS.*?audio_begin_playback.*?i2s\.play(?:WAV|MP3).*?audio_end_playback' "LittleFS audio playback must gate the amplifier for only the playback duration."
Assert-Contains $mpegCpp '(?s)audio_begin_playback.*?while\s*\(!plm_has_ended\(plm\)\).*?audio_end_playback' "MPEG audio must gate the amplifier for only the decode loop."
Assert-NotContains $halCpp 'digitalWrite\(AUDIO_AMP_CTRL,\s*HIGH\);\s*\r?\n\s*hal\.audio_init\(\)' "Callers must not enable the amplifier before audio_init succeeds."
Assert-NotContains $protocolCpp 'digitalWrite\(AUDIO_AMP_CTRL,\s*HIGH\);\s*\r?\n\s*hal\.audio_init\(\)' "Protocol wake paths must not pulse the amplifier before audio_init succeeds."
Assert-Contains $mainCpp 'esp_reset_reason\(\)' "Boot logs must include the reset reason for field diagnosis."
Assert-Contains $mainCpp 'heap_caps_get_free_size\(MALLOC_CAP_INTERNAL\)' "Boot logs must include free internal heap."
Assert-Contains $halCpp 'screen_buf\s*==\s*NULL' "LVGL draw-buffer allocation failure must be handled explicitly."
Assert-Contains $mainCpp '(?s)config_bootanimation.*?psram_ok.*?videoPlayer\.playBuffer' "Boot animation must be skipped when PSRAM initialization fails."
Assert-Contains $mainCpp 'psram_ok\s*=' "Boot must retain the PSRAM initialization result for downstream guards."
Assert-Contains $sdkconfig 'CONFIG_ESP_MAIN_TASK_STACK_SIZE=8192' "Startup config migration and JSON parsing require an 8192-byte main-task stack."

Write-Host "Boot resilience checks passed."
