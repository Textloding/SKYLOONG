param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

$ErrorActionPreference = "Stop"

function Assert-FileContains {
    param(
        [string]$Path,
        [string]$Pattern,
        [string]$Message
    )

    $content = Get-Content -LiteralPath $Path -Raw
    if ($content -notmatch $Pattern) {
        throw $Message
    }
}

function Assert-FileNotContains {
    param(
        [string]$Path,
        [string]$Pattern,
        [string]$Message
    )

    $content = Get-Content -LiteralPath $Path -Raw
    if ($content -match $Pattern) {
        throw $Message
    }
}

$halHeader = Join-Path $RepoRoot "main\include\hal.h"
$halSource = Join-Path $RepoRoot "main\hal.cpp"
$decoder = Join-Path $RepoRoot "main\mpegDecoder.cpp"

Assert-FileContains $halHeader "config_video_fit" "HAL must expose a persisted video fit setting for the web UI."
Assert-FileContains $halSource "video_fit" "The web API must expose and accept the video fit setting."
Assert-FileContains $halSource "cJSON_AddNumberToObject\(json,\s*`"screen_width`"" "The info API must expose the physical video target width."
Assert-FileContains $halSource "cJSON_AddNumberToObject\(json,\s*`"screen_height`"" "The info API must expose the physical video target height."

Assert-FileContains $decoder "plm_video_decode\(video\)" "Raw MPEG fallback playback should decode one frame at a time."
Assert-FileContains $decoder "plm_get_width\(plm\)" "Video playback must validate MPEG width before playback."
Assert-FileContains $decoder "plm_get_height\(plm\)" "Video playback must validate MPEG height before playback."
Assert-FileContains $decoder "Invalid MPEG stream" "Video playback must reject invalid MPEG streams without hanging."
Assert-FileContains $decoder "elapsed_ms\s*/\s*1000\.0f" "Muxed MPEG playback should advance by measured elapsed time for audio/video sync."
Assert-FileContains $decoder "fallback_frame_delay" "Muted MPEG playback should still pace frames from MPEG frame rate."
Assert-FileContains $decoder "plm_set_audio_enabled\(plm,\s*false\)" "Muted video playback must explicitly disable audio decoding."
Assert-FileContains $decoder "plm_set_audio_decode_callback\(plm,\s*my_audio_callback" "Video playback must route optional audio through the audio callback."
Assert-FileContains $decoder "frame->y\.width" "YUV conversion must use the MPEG luma plane stride."
Assert-FileContains $decoder "frame->cr\.width" "YUV conversion must use the MPEG chroma plane stride."
Assert-FileContains $decoder "my_video_callback\(NULL,\s*frame,\s*NULL\)" "Raw fallback frames decoded one at a time must still be drawn."
Assert-FileContains $decoder "frame->width\s*>\s*screenWidth" "Video playback must reject frames wider than the screen."
Assert-FileContains $decoder "frame->height\s*>\s*screenHeight" "Video playback must reject frames taller than the screen."
Assert-FileContains $decoder "tft\.fillScreen\(TFT_BLACK\)" "Video playback should clear the frame area/background before non-fullscreen playback."

Assert-FileNotContains $decoder "plm_decode\(plm,\s*1\)" "Video playback must not advance MPEG time in one-second jumps."
Assert-FileNotContains $decoder "980\s*-" "Video playback must not use the old fixed ~1 FPS delay."

Write-Host "video pipeline source checks passed"
