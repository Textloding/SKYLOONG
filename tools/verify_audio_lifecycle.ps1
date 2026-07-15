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
$protocol = Read-RepoFile "main\protocol.cpp"
$decoder = Read-RepoFile "main\mpegDecoder.cpp"
$i2sHeader = Read-RepoFile "components\arduino\libraries\ESP_I2S\src\ESP_I2S.h"
$i2sSource = Read-RepoFile "components\arduino\libraries\ESP_I2S\src\ESP_I2S.cpp"
$timerCallback = [regex]::Match($halSource, '(?s)void HAL::audio_amp_idle_timer_callback\(.*?(?=esp_err_t HAL::audio_init\(\))').Value
$systemTask = [regex]::Match($halSource, '(?s)static void task_systemctl\(.*?(?=void lcd_init\(\))').Value
$audioBegin = [regex]::Match($halSource, '(?s)bool HAL::audio_begin_playback\(\).*?(?=bool HAL::audio_configure_tx\()').Value
$audioInit = [regex]::Match($halSource, '(?s)esp_err_t HAL::audio_init\(\).*?(?=void HAL::audio_stop\(\))').Value
$audioStop = [regex]::Match($halSource, '(?s)void HAL::audio_stop\(\).*?(?=bool HAL::audio_begin_playback\(\))').Value
$keytoneStop = [regex]::Match($halSource, '(?s)void HAL::audio_stop_keytone\(uint32_t generation\).*?(?=void HAL::audio_stop\(\))').Value
$audioConfigure = [regex]::Match($halSource, '(?s)bool HAL::audio_configure_tx\(.*?(?=void HAL::audio_end_playback\(\))').Value
$audioEnd = [regex]::Match($halSource, '(?s)void HAL::audio_end_playback\(\).*?(?=void HAL::audio_shutdown\(\))').Value
$audioShutdown = [regex]::Match($halSource, '(?s)void HAL::audio_shutdown\(\).*?(?=void HAL::init\(\))').Value
$setVolume = [regex]::Match($halSource, '(?s)void HAL::setVolume\(.*?(?=void HAL::LOCKLV\(\))').Value
$requestKeytone = [regex]::Match($halSource, '(?s)void HAL::request_keytone\(\).*?(?=void HAL::send_sysctl\()').Value
$beginKeytone = [regex]::Match($halSource, '(?s)static bool beginKeytonePlayback\(.*?(?=static void endKeytonePlayback\()').Value
$endKeytone = [regex]::Match($halSource, '(?s)static void endKeytonePlayback\(.*?(?=static bool playWavWithAmplifier\()').Value
$keypress = [regex]::Match($protocol, '(?s)case PROTOCOL_TYPE_KEYPRESS:.*?(?=\s*default:)').Value
$playWav = [regex]::Match($i2sSource, '(?s)bool I2SClass::playWAV\(.*?(?=bool I2SClass::playMP3\()').Value
$playMp3 = [regex]::Match($i2sSource, '(?s)bool I2SClass::playMP3\(.*?(?=void I2SClass::stop\()').Value
$i2sEnd = [regex]::Match($i2sSource, '(?s)bool I2SClass::end\(\).*?(?=bool I2SClass::configureTX\()').Value
$i2sConfigure = [regex]::Match($i2sSource, '(?s)bool I2SClass::configureTX\(.*?(?=void I2SClass::setTxConfigureCallback\()').Value

Assert-Contains $halHeader '_audio_mutex' "HAL must expose a dedicated recursive audio mutex."
Assert-Contains $halHeader '_audio_owner' "HAL must track the task that owns the audio lifecycle lock."
Assert-Contains $halHeader '_audio_shutdown_requested' "HAL must publish shutdown intent before waiting for playback ownership."
Assert-Contains $halHeader 'std::atomic<TaskHandle_t>\s+_audio_owner' "Cross-core playback ownership checks must be atomic."
Assert-Contains $halHeader 'std::atomic_bool\s+keytone_play' "Cross-core keytone interruption state must be atomic."
Assert-Contains $halHeader 'std::atomic_bool\s+audio_ready' "Cross-core audio readiness checks must be atomic."
Assert-Contains $halHeader '_audio_stop_generation' "Stop requests issued during playback setup must be detected by generation."
Assert-Contains $halHeader 'portMUX_TYPE\s+_audio_stop_mux' "Playback owner publication and asynchronous I2S stop must share a short cross-core critical section."
Assert-Contains $halHeader '_audio_amp_idle_timer' "HAL must own a one-shot timer that closes the amplifier after the typing burst."
Assert-Contains $halHeader 'portMUX_TYPE\s+_audio_amp_mux' "Amplifier GPIO and mode transitions must be serialized across cores."
Assert-Contains $halHeader 'audio_amp_mode_t' "HAL must explicitly track off, playback, idle-hold, and shutdown amplifier modes."
Assert-Contains $halHeader '_audio_amp_gpio_high' "HAL must keep the GPIO latch mirror inside the amplifier state machine."
Assert-Contains $halHeader '_audio_amp_idle_deadline' "Stale timer callbacks must be rejected against the latest idle deadline."
Assert-Contains $halHeader '_queue_keytone' "Keytone requests need a dedicated single-slot queue so they cannot crowd out system events."
Assert-Contains $halHeader '_keytone_request_generation' "Keytone startup must detect a newer request without pretending that pending audio already owns I2S."
Assert-Contains $halHeader '_keytone_request_mutex' "Multiple keytone producer tasks must serialize generation allocation and queue publication."
Assert-Contains $halHeader 'request_keytone\(\)' "HAL must expose a coalescing keytone request API."
Assert-Contains $halHeader 'audio_begin_playback\(\)' "HAL must keep a complete playback ownership API."
Assert-Contains $halHeader 'audio_configure_tx\(' "HAL must serialize I2S reconfiguration with amplifier state."
Assert-Contains $halSource 'xSemaphoreCreateRecursiveMutex' "Audio lifecycle synchronization must use a recursive FreeRTOS mutex."
Assert-Contains $halSource 'audio_lock\(' "Audio lifecycle methods must acquire the dedicated audio mutex."
Assert-Contains $halSource 'audio_unlock\(' "Audio lifecycle methods must release the dedicated audio mutex."
Assert-Contains $audioInit 'audio_lock\(' "audio_init must serialize codec and I2S initialization."
Assert-Contains $audioStop 'audio_lock\(' "audio_stop must serialize I2S cancellation."
Assert-Contains $audioStop '(?s)portENTER_CRITICAL\(&_audio_stop_mux\).*?_audio_stop_generation\.fetch_add\(1\).*?target_owner\s*=\s*_audio_owner\.load\(\).*?i2s\.stop\(\).*?portEXIT_CRITICAL\(&_audio_stop_mux\)' "A stop request must target one published owner without crossing into its successor."
Assert-Contains $audioStop '(?s)target_owner\s*==\s*NULL\s*\|\|\s*target_owner\s*!=\s*current.*?return;' "A non-owner stop request must remain asynchronous instead of blocking behind long media."
Assert-Contains $keytoneStop '(?s)portENTER_CRITICAL\(&_audio_stop_mux\).*?_keytone_request_generation\.load\(\)\s*==\s*generation.*?keytone_play\.load\(\).*?_audio_stop_generation\.fetch_add\(1\).*?_audio_owner\.load\(\).*?i2s\.stop\(\).*?portEXIT_CRITICAL\(&_audio_stop_mux\)' "A keytone request must revalidate its generation, the active keytone, and its owner inside the stop critical section."
Assert-Contains $audioStop '(?s)_audio_stop_generation\.fetch_add\(1\).*?i2s\.stop\(\)' "Every stop request must publish a generation before cancelling I2S."
Assert-Contains $audioShutdown 'audio_lock\(' "audio_shutdown must serialize codec and I2S teardown."
Assert-Contains $setVolume 'audio_lock\(' "setVolume must serialize ES8311 register writes."
Assert-Contains $halSource 'AUDIO_DMA_TAIL_GUARD_MS\s*=\s*50' "Playback shutdown must define a 50 ms DMA tail guard."
Assert-Contains $halSource 'AUDIO_AMP_STARTUP_GUARD_MS\s*=\s*100' "Playback startup must allow the amplifier 100 ms to leave shutdown before PCM begins."
Assert-Contains $halSource 'AUDIO_AMP_IDLE_HOLD_MS\s*=\s*500' "Playback must keep the amplifier ready across a normal typing burst."
Assert-Contains $audioBegin 'audio_lock\(pdMS_TO_TICKS\(' "A busy media owner must not block the system-control task indefinitely."
Assert-NotContains $audioBegin 'audio_lock\(portMAX_DELAY\)' "Playback acquisition must use a finite wait."
Assert-Contains $audioBegin '(?s)audio_amp_start_playback\(.*?xTimerStop\(_audio_amp_idle_timer.*?delay\(AUDIO_AMP_STARTUP_GUARD_MS\).*?audio_amp_playback_ready\(\).*?return true' "Only a cold amplifier start may delay PCM; warm retriggers must reuse the active amplifier and reject stale timer callbacks."
Assert-Contains $audioConfigure '(?s)audio_amp_mute_for_reconfigure\(\).*?i2s\.configureTX.*?audio_amp_resume_after_reconfigure\(\).*?delay\(AUDIO_AMP_STARTUP_GUARD_MS\).*?audio_amp_playback_ready\(\)' "I2S format changes must remain owned, muted, guarded, and rechecked before PCM resumes."
Assert-Contains $audioConfigure '(?s)if\s*\(\s*!configured\s*\).*?audio_ready\.store\(false\).*?i2s\.end\(\)' "A failed I2S reconfiguration must invalidate and tear down the unusable channel."
Assert-Contains $audioEnd '(?s)delay\(AUDIO_DMA_TAIL_GUARD_MS\).*?audio_amp_begin_idle_hold' "Playback shutdown must wait for the final DMA samples before entering idle hold."
Assert-Contains $audioEnd 'xTimerReset\(_audio_amp_idle_timer,\s*0\)\s*==\s*pdPASS' "Playback completion must check whether the idle timer was actually armed."
Assert-Contains $audioEnd 'audio_amp_cancel_idle_hold' "A failed timer arm must fail closed without muting a newer playback generation."
Assert-Contains $audioEnd '(?s)recover_audio\s*=\s*!audio_ready\.load\(\).*?_audio_shutdown_requested\.load\(\).*?audio_unlock\(\).*?if\s*\(recover_audio.*?audio_init\(\)' "A fatal runtime reconfiguration failure must rebuild audio after releasing playback ownership."
Assert-Contains $audioShutdown '(?s)audio_amp_request_shutdown\(\).*?i2s\.stop\(\).*?audio_lock\(' "Audio shutdown must atomically publish intent and mute before waiting for playback ownership."
Assert-Contains $audioShutdown '(?s)audio_lock\(.*?audio_amp_shutdown_is_current\(shutdown_epoch\).*?audio_ready\.store\(false\)' "A stale concurrent shutdown must not tear down a newer audio generation."
Assert-NotContains $timerCallback 'audio_lock\(|xTimer(?:Reset|ChangePeriod|Stop|Start)\(|delay\(' "The timer daemon callback must never block or enqueue retries."
Assert-Contains $timerCallback 'audio_amp_finish_idle_hold' "The timer callback must only apply the current idle-hold deadline."
Assert-NotContains ($audioBegin + $audioConfigure + $audioEnd + $audioShutdown + $setVolume) 'digitalWrite\(AUDIO_AMP_CTRL' "Amplifier GPIO writes must only occur inside the serialized state helper."
Assert-Contains $halSource '_audio_owner\.store\(owner\)' "Playback must retain the published owner until audio_end_playback."
Assert-Contains $audioEnd 'audio_clear_owner\(\)' "Playback ownership must be released after the DMA tail drains."
Assert-Contains $halSource 'audio_stop' "Audio stop must participate in lifecycle synchronization."
Assert-Contains $decoder 'audio_begin_playback\(\)' "MPEG playback must acquire audio ownership before configuring/writing I2S."
Assert-Contains $decoder '(?s)audio_begin_playback\(\).*?hal\.audio_configure_tx' "MPEG I2S configuration must use the HAL-owned amplifier state machine."
Assert-NotContains $decoder 'digitalWrite\(AUDIO_AMP_CTRL' "MPEG playback must not bypass HAL amplifier state tracking."
Assert-Contains $decoder 'audio_end_playback\(\)' "MPEG playback must release audio ownership after the final frame."
Assert-Contains $i2sHeader 'void\s+clearStop\(\)' "I2S must expose an explicit way to clear an earlier asynchronous stop request."
Assert-Contains $i2sSource '(?s)void\s+I2SClass::clearStop\(\).*?_stop\.store\(false\)' "I2S clearStop must restore writes after a cancelled playback."
Assert-Contains $i2sHeader 'setTxConfigureCallback\(' "WAV and MP3 playback must delegate format changes to the HAL owner."
Assert-Contains $playWav 'configureTXForPlayback\(' "WAV format changes must use the registered HAL callback."
Assert-Contains $playMp3 'configureTXForPlayback\(' "MP3 format changes must use the registered HAL callback."
Assert-Contains $halSource '(?s)bool HAL::audio_publish_owner\(.*?portENTER_CRITICAL\(&_audio_stop_mux\).*?i2s\.clearStop\(\).*?_audio_stop_generation\.load\(\)\s*==\s*stop_generation.*?_audio_owner\.store\(owner\).*?portEXIT_CRITICAL\(&_audio_stop_mux\)' "Owner publication must not erase a stop racing with playback setup."
Assert-Contains $halSource '(?s)void HAL::audio_clear_owner\(\).*?portENTER_CRITICAL\(&_audio_stop_mux\).*?_audio_owner\.store\(NULL\).*?portEXIT_CRITICAL\(&_audio_stop_mux\)' "Owner release must serialize with asynchronous stop targeting."
Assert-Contains $audioBegin '(?s)stop_generation\s*=\s*_audio_stop_generation\.load\(\).*?audio_publish_owner\(current,\s*stop_generation\)' "Every new playback scope must publish ownership against its captured stop generation."
Assert-Contains $audioBegin '(?s)_audio_shutdown_requested.*?audio_unlock\(\).*?return false' "New playback must be rejected while audio shutdown is pending."
Assert-NotContains $playWav '_stop\s*=\s*false' "WAV playback must not erase a stop request issued after playback ownership was acquired."
Assert-NotContains $playMp3 '_stop\s*=\s*false' "MP3 playback must not erase a stop request issued after playback ownership was acquired."
Assert-Contains $audioShutdown '(?s)i2s\.stop\(\).*?gif_vid_stop\s*=\s*true.*?audio_lock\(' "Audio shutdown must cancel looping media before waiting for playback ownership."
Assert-Contains $audioShutdown '(?s)audio_amp_request_shutdown\(\).*?i2s\.stop\(\)' "Audio shutdown intent must be serialized before issuing asynchronous cancellation."
Assert-Contains $audioShutdown 'pdMS_TO_TICKS\(' "Audio shutdown must use a finite lock wait after requesting cancellation."
Assert-Contains $audioShutdown '(?s)if\s*\(\s*!audio_lock.*?audio_amp_mark_shutdown_complete\(shutdown_epoch\).*?return' "A timed-out shutdown must leave a recoverable completed cancellation generation."
Assert-Contains $audioConfigure 'data_width\s*!=\s*I2S_DATA_BIT_WIDTH_16BIT' "HAL must reject sample widths that do not match the ES8311 16-bit configuration."
Assert-Contains $audioConfigure '(?s)es8311_sample_frequency_config\(.*?i2s\.configureTX' "I2S and ES8311 sample-frequency changes must be applied in one owned transition."
Assert-Contains $audioConfigure '(?s)if\s*\(codec_err\s*==\s*ESP_ERR_INVALID_ARG\).*?audio_amp_force_off\(\).*?audio_unlock\(\).*?return false' "An unsupported file sample rate must reject only that playback without disabling the known-good audio channel."
Assert-Contains $i2sHeader 'std::atomic_bool\s+_stop' "Cross-core playback cancellation must use an atomic stop flag."
Assert-Contains $i2sHeader '_tx_chan_enabled' "I2S must track whether its TX channel is RUNNING before using the format fast path."
Assert-Contains $i2sConfigure '(?s)_tx_chan_enabled.*?tx_sample_rate.*?i2s_channel_disable.*?_tx_chan_enabled\s*=\s*false.*?i2s_channel_enable.*?_tx_chan_enabled\s*=\s*true' "I2S reconfiguration must only cache a usable format while the channel is RUNNING."
Assert-Contains $i2sEnd '(?s)i2s_channel_disable\(tx_chan\).*?ESP_ERR_INVALID_STATE.*?i2s_del_channel\(tx_chan\).*?tx_chan\s*=\s*NULL' "I2S teardown must delete a channel that is already in READY state after failed reconfiguration."
Assert-Contains $i2sEnd '(?s)if\s*\(tx_chan\s*!=\s*NULL\s*\|\|\s*rx_chan\s*!=\s*NULL\).*?return false;.*?perimanClearPinBus' "I2S teardown must preserve peripheral ownership when a live channel could not be deleted."
Assert-Contains $i2sSource 'bus->tx_chan\s*!=\s*NULL\s*\|\|\s*bus->rx_chan\s*!=\s*NULL' "Peripheral detach must account for both TX and RX channels."
Assert-Contains $i2sSource '(?s)bool I2SClass::i2sDetachBus\(.*?return bus->end\(\)' "Peripheral detach must report a channel teardown failure instead of releasing live pins."
Assert-Contains $halSource '_queue_keytone\s*=\s*xQueueCreate\(1,\s*sizeof\(uint32_t\)\)' "The pending keytone queue must retain exactly the newest request generation."
Assert-Contains $requestKeytone '(?s)xSemaphoreTake\(_keytone_request_mutex,\s*portMAX_DELAY\).*?_keytone_request_generation\.fetch_add\(1\).*?audio_stop_keytone\(request\).*?xQueueOverwrite\(_queue_keytone.*?xSemaphoreGive\(_keytone_request_mutex\)' "Concurrent producers must atomically publish one generation, conditionally stop its predecessor, and overwrite the pending slot."
Assert-Contains $beginKeytone '(?s)_keytone_request_generation\.load\(\)\s*!=\s*generation.*?audio_begin_playback\(\).*?keytone_play\.store\(true\).*?_keytone_request_generation\.load\(\)\s*!=\s*generation.*?audio_end_playback\(\)' "Keytone startup must reject superseded requests both before and after acquiring audio ownership."
Assert-Contains $endKeytone '(?s)keytone_play\.store\(false\).*?audio_end_playback\(\)' "A completed keytone must clear its active flag before releasing ownership to another audio user."
Assert-Contains $systemTask '(?s)xQueueReceive\(hal\._queue,\s*&event,\s*0\).*?xQueueReceive\(hal\._queue_keytone' "System events must be checked before pending keytones so shutdown and sleep cannot starve."
Assert-Contains $keypress 'hal\.request_keytone\(\)' "Every debounced keypress must retain the newest pending keytone request."
Assert-NotContains $keypress 'audio_stop\(\)' "Keytone interruption and generation publication must be one operation inside request_keytone."
Assert-Contains $systemTask 'hal\.audio_ready\.load\(\)' "The system task must observe audio readiness atomically before consuming a keytone request."
Assert-Contains $systemTask 'uint32_t\s+keytone_request' "The system task must dequeue a complete keytone request generation."
Assert-NotContains $keypress 'send_sysctl\(EVENT_KB_KEYPRESS' "Keytones must not consume slots in the shared system-event queue."

Write-Host "Audio lifecycle source checks passed."
