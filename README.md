# SKYLOONG Screen Module Firmware

SKYLOONG keyboard screen module firmware based on ESP32-S3 and ESP-IDF 5.1.4.

This fork focuses on a cleaner daily-use experience: a modern web management
console, easier Wi-Fi setup, configurable media playback, screen audio controls,
weather city selection, and safer upload presets for the 320 x 240 screen.

![Management overview](docs/images/console-overview.png)

## What It Does

- Manage the SKYLOONG/GK87-style 320 x 240 keyboard screen from a browser.
- Upload pictures, GIFs, videos, and custom key tones.
- Choose video layout: full image with black bars, or fullscreen crop.
- Toggle video sound and device volume from the management console.
- Configure Wi-Fi from the browser and keep multiple saved networks for auto reconnect.
- Configure weather city without exposing the weather API key in the web UI.
- Enable or disable screen apps such as time, weather, system info, APS, pictures, and videos.
- Flashable ESP32-S3 firmware built with ESP-IDF v5.1.4.

## Management Console

The built-in web console is designed for desktop and mobile screens. It uses a
glass-style dark interface with quick status cards, responsive navigation, and
task-focused pages for media, display, network, and system settings.

### Overview

![Overview page](docs/images/console-overview.png)

The overview page shows device status, current IP, storage usage, saved Wi-Fi
count, and quick actions for common tasks.

### Media Upload

![Media page](docs/images/console-media.png)

The media page supports image uploads, video/GIF upload, key tone upload, and
file deletion. MP4 and GIF sources are transcoded in the browser before upload:

- Output canvas: `320 x 240`, matching the physical screen.
- Frame rate: MPEG1-compatible `24 FPS`.
- Video: low bitrate MPEG1 with no B frames.
- Audio: optional MP2 mono audio, or removed entirely when video sound is off.
- Fullscreen mode fills the screen but may crop edges.
- Complete-display mode preserves the full picture and adds black bars if needed.

For the smoothest playback, turn off video sound before uploading large or 4K
source files. Existing videos already stored on the screen will not be changed;
re-upload the original source file to apply the new transcode settings.

### System Settings

![System page](docs/images/console-system.png)

The system page controls volume, video sound, time zone, language, weather city,
PC monitor target, and device reboot.

## Flashing Firmware

### Requirements

- SKYLOONG/GK87 ESP32-S3 screen module.
- USB data cable.
- Windows is recommended for this branch.
- ESP-IDF v5.1.4 installed at `%USERPROFILE%\esp\esp-idf`.
- ESP-IDF tools installed at `%USERPROFILE%\.espressif`.
- Git.

The screen usually appears as an Espressif USB serial/JTAG device:

```text
USB VID:PID=303A:1001
USB serial device (COM3)
```

Your COM port may be different.

### Build

Open an ESP-IDF terminal, or run the export script before building:

```bat
cd path\to\SKYLOONG
call "%USERPROFILE%\esp\esp-idf\export.bat"
idf.py build
```

On Windows, if the project path contains Chinese characters or is very long,
copy the repository to a short path such as `C:\s` before building:

```powershell
robocopy "C:\path\to\SKYLOONG" C:\s /MIR /XD .git build managed_components web_new\__pycache__ web_new\_mockfs
cd C:\s
cmd /d /c 'call "%USERPROFILE%\esp\esp-idf\export.bat" && idf.py build'
```

### Flash

Find the port:

```bat
python -m serial.tools.list_ports -v
```

Flash the firmware:

```bat
idf.py -p COM3 flash
```

Replace `COM3` with your actual port. A successful flash ends with verified
hashes and a hard reset:

```text
Hash of data verified.
Leaving...
Hard resetting via RTS pin...
Done
```

### Common Flashing Notes

- If only `COM1` is visible, the screen is not currently exposed as a flashable
  ESP32-S3 serial device. Replug the USB cable or enter download mode.
- Use a data cable, not a charge-only cable.
- If flashing fails while connecting, hold the screen boot/download button if
  your module exposes one, then retry `idf.py -p COMx flash`.
- Do not flash to a port unless it matches the ESP32-S3 device.

## Web Console Development

Editable web source lives in:

```text
web_new/
```

The firmware-embedded web files live in:

```text
web/
main/include/webserver/
```

After editing `web_new`, sync the files to `web`, then regenerate embedded
headers:

```powershell
Copy-Item -Force web_new\index.js web\index.js
Copy-Item -Force web_new\index.css web\index.css
Copy-Item -Force web_new\index.html web\index.html
& 'C:\Program Files\Git\bin\bash.exe' updateWeb.sh
```

Then rebuild and flash:

```bat
idf.py build
idf.py -p COM3 flash
```

## Verification

The `tools/` directory contains lightweight source checks used while developing
this fork:

```powershell
node --check web_new\index.js
node --check web\index.js
powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_web_video_transcode.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_web_dirty_state.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_video_pipeline.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_wifi_pipeline.ps1
```

Recommended full check before release:

```bat
idf.py build
idf.py -p COM3 flash
```

## Repository Layout

```text
main/                    ESP32-S3 firmware source
main/include/webserver/  Generated embedded web assets
web/                     Web assets embedded into firmware
web_new/                 Editable modern management console
components/              ESP-IDF/Arduino/LVGL/TFT/audio dependencies
tools/                   Source verification scripts
docs/images/             README screenshots
```

## License

Project code and local modifications in this repository are released under the
0BSD license. See [LICENSE](LICENSE).

Third-party components and vendored dependencies keep their original licenses;
check the corresponding component directories for their license files.
