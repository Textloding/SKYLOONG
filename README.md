# Develop document authorization instructions

* If your use/modification/distribution of this development document is a business act of the company, please seek official authorization from Skyloong(http://www.skyloong.com.cn).
* Individual enthusiasts of non-commercial activities are free to use/modify this development document.
* If you have any questions, you can also contact: zhouziran@jikedingzhi.com
----------------------------------------------------------------------------------------------------

# ESP32_screen_module
![esp32_screen_module](https://i.imgur.com/TjqTpUu.jpeg)

## Skyloong  keyboard screen module Feature
*  Supported by esp32 chip;
*  It can be replace by standard mechenical switch easily;
*  Display date, time, keyboard stage, Battery level, etc; 
*  Display your favorite pictures, GIFs, etc;
*  Show the weather in your city and a concise living guide;
*  Displays performance information about computers with a specified IP address in the same LAN;
*  APS tools can help you know your typing speed.
---------------------------------------------------------------------------------------------------

# Firmware Compilation Guide

## What You Need

Hardware

* An **GK87_screen module**.
* **USB cable** - USB A / Type-C .
* **Computer** running Windows, Linux, or macOS.

Software

To start using ESP-IDF on **GK87_screen module**, install the following software:

* **Toolchain** to compile code for GK87_screen module
* **Build tools** - CMake and Ninja to build a full **Application** for GK87_screen module
* **ESP-IDF** that essentially contains API (software libraries and source code) for GK87_screen module and scripts to operate the **Toolchain**

# Installation
## For Mac or Linux : 
**Install IDE** - Install VSCode Extension as below:
1. Install the following [ESP-IDF Prerequisites](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html#step-1-install-prerequisites). 
2. Download and install [Visual Studio Code](https://code.visualstudio.com/).
3. Open the **Extensions** view by clicking on the Extension icon in the Activity Bar on the side of Visual Studio Code or the **View: Extensions** command (shortcut: <kbd>⇧</kbd> <kbd>⌘</kbd> <kbd>X</kbd> or <kbd>Ctrl+Shift+X</kbd>.)
4. Search for [ESP-IDF Extension](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension).
5. Install the extension.
6. (OPTIONAL) Press <kbd>F1</kbd> and type **ESP-IDF: Select where to Save Configuration Settings**, which can be User Settings, Workspace Settings or Workspace Folder Settings. Please take a look at [Working with multiple projects](../MULTI_PROJECTS.md) for more information. Default is User settings.
7. In Visual Studio Code, select menu "View" and "Command Palette" and type [configure esp-idf extension]. After, choose the **ESP-IDF: Configure ESP-IDF Extension** option. You can also choose where to save settings in the setup wizard.
8. Now the setup wizard window will be shown with several setup options: **Express**, **Advanced** or **Use Existing Setup**.
> **NOTE**: **Use Existing Setup** setup mode option is only shown if:
>
> - `esp-idf.json` is found in the current `idf.toolsPath` . This file is generated when you install ESP-IDF with the [IDF-ENV](https://github.com/espressif/idf-env).
> - ESP-IDF is found in `idf.espIdfPath` , `IDF_PATH` environment variable `$HOME/esp/esp-idf` on MacOS/Linux
> - ESP-IDF Tools and ESP-IDF Python virtual environment for the previous ESP-IDF are found in `idf.toolsPath`, `IDF_TOOLS_PATH` environment variable `$HOME\.espressif` on MacOS/Linux.

9. Choose **Express** for the fastest option (or **Use Existing Setup** if ESP-IDF is already installed)
10. If you choose **Express** setup mode:
    - Pick V5.1.3 ESP-IDF version to download (or find the ESP-IDF in your system) and the python executable to create the virtual environment.
    - Choose the location for ESP-IDF Tools and python virtual environment (also known as `IDF_TOOLS_PATH`) which is `$HOME\.espressif` by default.
      > **NOTE:** Make sure that `IDF_TOOLS_PATH` doesn't have any spaces to avoid any build issues.

11. The user will see a page showing the setup progress status showing ESP-IDF download progress, ESP-IDF Tools download and install progress as well as the creation of a python virtual environment.

12. (OPTIONAL) If the user have chosen the **Advanced** option, after ESP-IDF is downloaded and extracted, select to either download ESP-IDF Tools or manually provide each ESP-IDF tool absolute path and required environment variables.
    > **NOTE:** Consider that `IDF_PATH` requires each ESP-IDF tool to be of the version described in `IDF_PATH`/tools/tools.json.
    > If it is desired to use a different ESP-IDF tool version, check [JSON Manual Configuration](../SETUP.md#JSON-Manual-Configuration)

13. (OPTIONAL) If the user has chosen the **Advanced** mode and selected to manually provide each ESP-IDF tool absolute path, please enter the executable container directory for each binary as shown below:
    > **NOTE:** Check [JSON Manual Configuration](../SETUP.md#JSON-Manual-Configuration) for more information.

14. If everything is installed correctly, the user will see a message that all settings have been configured. You can start using the extension.

> **NOTE**: > The advance mode allows the user to choose to use existing ESP-IDF tools by manually entering each ESP-IDF tool absolute path. Again, if chose an ESP-IDF version < 5.0, make sure each ESP-IDF tool path doesn't have any spaces, since they were no suported in previous versions..

15. Now that the extension setup is finally done, check the [Basic use](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/basic_use.md) to learn how to use the SDK Configuration editor, build, flash and monitor your Espressif device.

> **NOTE**: Visual Studio Code has many places where to set configuration settings. This extension uses the `idf.saveScope` configuration setting to determine where to save settings, Global (User Settings), Workspace and WorkspaceFolder. Please review [vscode settings precedence](https://code.visualstudio.com/docs/getstarted/settings#_settings-precedence).

> **NOTE:** the setup wizard will install ESP-IDF Python packages and ESP-IDF debug adapter (`EXTENSION_PATH`/esp_debug_adapter/requirements.txt) python packages. Make sure that if using an existing python virtual environment that installing these packages doesn't affect your virtual environment. The `EXTENSION_PATH` is:
- Linux & MacOSX: `$HOME/.vscode/extensions/espressif.esp-idf-extension-VERSION`

# Compiling the Project

`idf.py build`

... will compile app, bootloader and generate a partition table based on the config.

# Flashing the Project

When the build finishes, it will print a command line to use esptool.py to flash the chip. However you can also do this automatically by running:

`idf.py -p PORT flash`

Replace PORT with the name of your serial port (`/dev/ttyUSB0` on Linux, or `/dev/cu.usbserial-X` on MacOS). If the `-p` option is left out, `idf.py` flash will try to flash the first available serial port.

This will flash the entire project (app, bootloader and partition table) to a new chip. The settings for serial port flashing can be configured with `idf.py menuconfig`.

You don't need to run `idf.py build` before running `idf.py flash`, `idf.py flash` will automatically rebuild anything which needs it.

# Compiling & Flashing Only the App

After the initial flash, you may just want to build and flash just your app, not the bootloader and partition table:

* `idf.py app` - build just the app.
     
* `idf.py app-flash` - flash just the app.

`idf.py app-flash` will automatically rebuild the app if any source files have changed.

(In normal development there's no downside to reflashing the bootloader and partition table each time, if they haven't changed.)

# Erasing Flash

The `idf.py flash` target does not erase the entire flash contents. However it is sometimes useful to set the device back to a totally erased state, particularly when making partition table changes or OTA app updates. To erase the entire flash, run `idf.py erase-flash`.

This can be combined with other targets, ie `idf.py -p PORT erase-flash flash` will erase everything and then re-flash the new app, bootloader and partition table.

