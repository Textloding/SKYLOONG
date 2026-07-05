param(
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

$ErrorActionPreference = "Stop"

function Read-Text($path) {
    return [System.IO.File]::ReadAllText($path)
}

function Assert-Contains($text, $pattern, $message) {
    if ($text -notmatch $pattern) {
        throw $message
    }
}

function Assert-NotContains($text, $pattern, $message) {
    if ($text -match $pattern) {
        throw $message
    }
}

$web = Read-Text (Join-Path $RepoRoot "web_new\index.js")

Assert-Contains $web 'VIDEO_TARGET_W\s*=\s*SCREEN_W' "Uploaded video must render to the full 320px screen width."
Assert-Contains $web 'VIDEO_TARGET_H\s*=\s*SCREEN_H' "Uploaded video must render to the full 240px screen height."
Assert-Contains $web 'VIDEO_TARGET_FPS\s*=\s*24' "Video must use an MPEG1-compatible frame rate for firmware playback."
Assert-Contains $web 'VIDEO_BITRATE\s*=\s*"320k"' "Video bitrate must be low enough for ESP32-S3 software playback."
Assert-Contains $web 'VIDEO_MAXRATE\s*=\s*"420k"' "Video maxrate must be capped to avoid playback spikes."
Assert-Contains $web 'VIDEO_AUDIO_BITRATE\s*=\s*"32k"' "Video audio bitrate must be low enough for simultaneous software decode."
Assert-Contains $web 'VIDEO_AUDIO_CHANNELS\s*=\s*"1"' "Video audio should be mono to reduce MP2 decode work."
Assert-Contains $web 'flags=fast_bilinear' "4K downscaling must use a fast scaler before upload."
Assert-Contains $web '"-bf",\s*"0"' "MPEG output should avoid B frames for lighter firmware decode."
Assert-Contains $web 'const\s+audioWanted\s*=\s*!!state\.info\?\.video_audio' "Video upload must follow the user's video sound setting."
Assert-Contains $web 'audioWanted\s*\?' "FFmpeg arguments must conditionally include audio."
Assert-Contains $web '"-an"' "Videos uploaded with sound disabled must not carry an audio stream."
Assert-Contains $web 'data-video-audio' "The video media page must expose the video sound toggle."
Assert-Contains $web 'video-tradeoff-note' "The media page must warn users about fullscreen and smoothness tradeoffs."

Assert-NotContains $web 'fps=15' "15 FPS is not an MPEG1-compatible frame rate for this firmware decoder."
Assert-NotContains $web 'VIDEO_TARGET_W\s*=\s*240' "Do not shrink video below screen width; it will not fill the display."
Assert-NotContains $web 'VIDEO_TARGET_H\s*=\s*180' "Do not shrink video below screen height; it will not fill the display."
Assert-NotContains $web '"650k"' "650k video bitrate is too heavy for reliable ESP32-S3 playback."
Assert-NotContains $web '"900k"' "900k video maxrate can cause playback stalls."
Assert-NotContains $web '"-ac",\s*"2"' "Stereo MP2 audio is too heavy for reliable simultaneous video playback."

Write-Host "web video transcode source checks passed."
