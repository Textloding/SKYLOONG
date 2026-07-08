# SKYLOONG ESP32-S3 屏幕固件

这是面向 SKYLOONG/GK87 系列 320 x 240 彩屏键盘屏幕模块的 ESP32-S3 固件。项目基于 ESP-IDF v5.1.4 和 Arduino 组件开发，提供屏幕小应用、网页管理后台、素材上传、Wi-Fi 配网、音量控制、天气、电脑监控、番茄钟等功能。

本仓库是在原始项目基础上的体验优化版，重点改善了网页管理后台、Wi-Fi 自动重连、视频/GIF 上传转码、声音设置、番茄钟和长期运行后的管理页可访问性。

![管理台总览](docs/images/console-overview.png)

## 当前版本

- 主分支面向普通用户使用，包含现代化管理台、Wi-Fi 自动重连、视频转码、番茄钟、键盘宠物和网页服务稳定性优化。
- 详细变更见 [2026-07-08 版本说明](docs/releases/2026-07-08.md)。
- 不想配置 ESP-IDF 的用户，推荐使用独立刷机工具：[Textloding/skyloong-flasher](https://github.com/Textloding/skyloong-flasher)。

## 功能概览

- 现代化网页管理后台，支持电脑和手机浏览器自适应访问。
- 支持图片、GIF、MP4、MPEG 视频和自定义音效上传。
- 针对 320 x 240 屏幕优化视频转码，可选择“完整显示”或“铺满屏幕”。
- 支持设备音量、视频声音开关、内置/自定义按键音和番茄钟提醒音设置。
- 支持多个 Wi-Fi 凭据保存和开机自动联网。
- 天气城市可手动设置，也可由浏览器自动识别后写入设备。
- 天气 API Key 不在管理后台直接展示，降低误操作风险。
- 支持手速监测、天气、电脑监控、图片相册、视频/GIF、番茄钟等屏幕页面。
- 番茄钟为独立小应用，支持专注/短休息/长休息、自定义提醒音、主题、远程重置当前倒计时和重置轮次。
- 提供 `/health`、`/info` 等接口用于排查设备联网和网页服务状态。

## 管理台展示

管理台采用深色毛玻璃风格，入口清晰，适合桌面端和手机端操作。管理台是固件内置页面，刷入设备后直接由屏幕模块提供网页服务。

### 总览页

![总览页](docs/images/console-overview.png)

总览页展示设备连接状态、当前 IP、存储空间、已保存 Wi-Fi 数量、屏幕预览和常用功能入口。

### 媒体页

![媒体页](docs/images/console-media.png)

媒体页用于上传图片、视频/GIF 和音效文件。上传 MP4 或 GIF 时，浏览器会先把素材转成更适合 ESP32-S3 解码的 MPEG 视频，再上传到设备。

视频相关策略：

- 屏幕物理分辨率：`320 x 240`。
- 默认输出：`320 x 240`、MPEG1 视频。
- 帧率：面向小屏流畅度优化，避免直接播放 4K/高码率源文件。
- “完整显示”：保留完整画面，比例不一致时补黑边。
- “铺满屏幕”：占满整块屏幕，比例不一致时裁切边缘。
- “播放视频声音”：开启后会尝试保留音轨并转成 MP2 单声道；关闭后上传的视频为静音，更省性能。

提示：已经上传到屏幕里的旧视频不会自动重新转码。修改视频适配方式或声音开关后，需要重新上传原始视频/GIF。

### 系统页

![系统页](docs/images/console-system.png)

系统页可以设置时区、语言、设备音量、视频声音、番茄钟、天气城市、电脑监控地址和设备重启。

## 适用设备

本固件面向使用 ESP32-S3、320 x 240 彩屏和 ES8311 音频芯片的 SKYLOONG/GK87 类屏幕模块。常见刷机串口特征如下：

```text
USB VID:PID = 303A:1001
设备名称可能显示为 USB Composite Device、USB JTAG/serial debug unit 或 USB 串行设备
串口示例：COM3
```

不同电脑上的端口号可能不同，刷机前必须以实际扫描结果为准。

## 刷固件前准备

### 推荐方式：使用一键刷机工具

普通用户建议优先使用独立刷机工具，不需要手动输入 `idf.py flash`：

```text
https://github.com/Textloding/skyloong-flasher
```

刷机工具支持选择本地 zip、输入 GitHub 仓库链接或分支链接，并在界面里显示下载、解析、构建、设备扫描和刷机进度。

下面的 ESP-IDF 命令适合开发者或需要自行编译固件的用户。

### 必备硬件

- SKYLOONG/GK87 ESP32-S3 屏幕模块。
- 能传输数据的 USB 线，不要只用充电线。
- Windows 电脑更推荐；Linux/macOS 也可以，但命令需要按系统调整。

### 必备软件

- Git。
- Python 3。
- ESP-IDF v5.1.4。
- ESP-IDF 对应工具链。

推荐按 Espressif 官方方式安装 ESP-IDF v5.1.4。Windows 默认路径通常类似：

```text
C:\Users\<用户名>\esp\esp-idf
C:\Users\<用户名>\.espressif
```

## 获取代码

```powershell
git clone https://github.com/Textloding/SKYLOONG.git
cd SKYLOONG
```

如果需要切到本固件使用的分支：

```powershell
git switch main
```

## 编译固件

### 方法一：在 ESP-IDF 终端中编译

打开 “ESP-IDF PowerShell” 或 “ESP-IDF Command Prompt”，进入仓库目录：

```powershell
cd C:\path\to\SKYLOONG
idf.py build
```

编译成功后会在 `build` 目录生成固件产物。

### 方法二：普通 PowerShell 中加载 ESP-IDF 环境

如果没有打开 ESP-IDF 专用终端，可以手动加载环境：

```powershell
cd C:\path\to\SKYLOONG
. "$env:USERPROFILE\esp\esp-idf\export.ps1"
idf.py build
```

### 路径注意事项

ESP-IDF、CMake 和部分工具对中文路径、空格路径、过长路径不够友好。如果编译遇到莫名错误，建议把仓库复制到短路径，例如：

```powershell
C:\s
```

然后在短路径中重新编译：

```powershell
cd C:\s
idf.py build
```

## 刷入固件

### 1. 连接设备

用 USB 数据线把屏幕模块连接到电脑。等待系统识别串口。

### 2. 查看串口号

```powershell
python -m serial.tools.list_ports -v
```

找到类似 `VID:PID=303A:1001`、`USB JTAG/serial debug unit` 或 `USB Composite Device` 的设备，并记录对应端口，例如 `COM3`。

### 3. 刷入

把下面命令中的 `COM3` 换成你的实际端口：

```powershell
idf.py -p COM3 flash
```

刷入成功时，终端末尾通常会看到：

```text
Hash of data verified.
Leaving...
Hard resetting via RTS pin...
Done
```

刷机完成后设备会自动重启。

## 第一次进入管理后台

### 情况一：设备已经保存过 Wi-Fi

设备启动后会尝试自动连接已保存的 Wi-Fi。联网成功后，屏幕会显示当前局域网 IP。用电脑或手机访问：

```text
http://屏幕显示的IP/
```

例如：

```text
http://192.168.1.186/
```

### 情况二：设备还没有配置 Wi-Fi

设备会进入配网模式并开启热点：

```text
SKYLOONG 4.0 Screen
```

用手机或电脑连接这个热点后，打开：

```text
http://192.168.4.1/wifi
```

选择 2.4GHz Wi-Fi，输入密码并提交。提交后设备会切换到新的局域网 IP。此时请等待屏幕显示新 IP，再用新 IP 打开管理后台。

### 重要说明：127.0.0.1 不是设备后台

开发时可能会使用本地调试地址：

```text
http://127.0.0.1:5173/
```

这个地址只表示电脑本机的开发服务器，不是刷进屏幕后的设备管理后台。普通用户刷机后应访问屏幕显示的局域网 IP，例如 `http://192.168.1.186/`。

## 日常使用

### 切换屏幕页面

屏幕页面切换沿用键盘固件提供的组合键，一般为：

```text
fn + ~
```

具体组合键可能随键盘型号或键盘固件而变化，请以你的键盘说明为准。

### 上传图片

进入管理台“媒体”页，选择图片上传。网页会按屏幕比例进行预览和裁剪，适合用作屏幕图片或相册内容。

### 上传视频或 GIF

进入“媒体”页，选择 MP4 或 GIF。网页会先加载浏览器端转码器，然后转成适合设备的小尺寸 MPEG。

建议：

- 优先使用短视频片段。
- 源文件可以是高清或 4K，但上传过程会压缩到 `320 x 240`。
- 如果播放卡顿，关闭“播放视频声音”后重新上传。
- 如果想画面铺满屏幕，选择“铺满屏幕”；这可能裁掉部分边缘。
- 如果想保留完整画面，选择“完整显示”；这可能出现黑边。

### 声音和音量

屏幕模块带音频输出能力。本固件支持：

- 系统页调节设备音量。
- 设置视频是否播放声音。
- 上传并试听按键音。
- 使用内置按键音：内置 1、内置 2、内置 3、弹跳三连、篮球律动、趣味上扬。
- 设置番茄钟提醒音。

注意：视频声音会占用更多解码和播放资源。若视频画面或声音卡顿，优先关闭视频声音并重新上传视频。

按键音说明：

- “关闭”会禁用按键音。
- “自定义”会使用媒体页上传的音频文件。
- 新增的“弹跳三连”“篮球律动”“趣味上扬”为固件内置轻量音效，不依赖 LittleFS 文件。
- 内置音效存放在固件只读区，避免占用运行时 DRAM；过大的自定义音频仍会占用设备存储空间。

### 番茄钟

番茄钟是独立屏幕小应用，不依赖天气、手速检测或视频页面。

支持设置：

- 是否启用番茄钟页面。
- 专注时长。
- 短休息时长。
- 长休息时长。
- 每几轮进入一次长休息。
- 倒计时主题。
- 内置提醒音或自定义提醒音。
- 在电脑端重置当前倒计时。
- 在电脑端重置轮次，重置后会回到第 1 轮专注并从完整专注时间重新开始。

倒计时结束后，屏幕显示 `00:00` 并播放提醒音。用户需要按：

```text
fn + ~
```

确认停止提醒音，并进入下一段倒计时。

这样设计是为了避免倒计时结束后自动切走导致用户错过提醒。

倒计时显示说明：

- 番茄钟圆环按当前剩余毫秒计算进度，比只按整秒刷新更平滑、更准确。
- 圆环按 `320 x 240` 屏幕重新布局，时间数字与顶部状态文字保持分离。
- 管理台的“重置当前倒计时”只重置当前专注/休息段，不改变已完成轮次。
- 管理台的“重置轮次”会停止当前提醒音，并回到第 1 轮专注。

### 天气

系统页支持设置天气城市。可手动输入城市，也可点击自动识别。

自动识别依赖浏览器定位或网络 IP 定位，可能受浏览器权限、网络环境、代理和第三方定位服务影响。识别不准确时，请手动选择或输入城市。

### 电脑监控

电脑监控需要设备和电脑处于同一局域网，并在管理台中填写电脑 IP 和端口。电脑端监控服务不在本 README 中展开，请以配套工具说明为准。

## 联网和管理台访问注意事项

### 启动后需要等待

设备上电或刷机后，屏幕需要完成启动、加载小应用、连接 Wi-Fi 并启动网页服务。这个过程可能需要 10 到 60 秒。

如果刚开机访问管理台很慢、打不开或只看到背景，请先等待半分钟到一分钟，再刷新页面。

### 切换 Wi-Fi 后 IP 可能变化

在管理台提交新的 Wi-Fi 后，设备会断开旧网络并连接新网络。原来的网页地址可能失效。请看屏幕显示的新 IP，再重新打开。

### 必须在同一局域网

电脑或手机必须和屏幕处在同一个局域网，才能通过 `http://屏幕IP/` 访问管理后台。

常见问题：

- 手机使用蜂窝网络时访问不到屏幕。
- 电脑连接有线网络，屏幕连接另一个隔离 Wi-Fi 时访问不到。
- 路由器开启 AP 隔离、访客网络隔离时访问不到。
- 公司、学校、酒店网络可能禁止设备互访。

### 只剩背景或加载很慢

可以按顺序排查：

1. 等待 30 到 60 秒后刷新。
2. 确认访问的是屏幕 IP，不是 `127.0.0.1:5173`。
3. 确认电脑和屏幕在同一网络。
4. 浏览器强制刷新，或在地址后加版本参数，例如 `http://屏幕IP/?v=latest`。
5. 在管理台可进入时点击“重启设备”，或断电重插。
6. 如果长期运行后仍频繁出现，请通过串口查看日志，并提交 issue。

### 健康检查接口

如果能访问设备 IP，但页面异常，可以直接打开：

```text
http://屏幕IP/health
http://屏幕IP/info
```

`/health` 可查看网页服务是否运行、Wi-Fi 是否连接、剩余堆内存等信息。`/info` 可查看当前设置状态。

## 常见刷机问题

### 找不到串口

- 换一根 USB 数据线。
- 换电脑 USB 口。
- 确认线材支持数据传输。
- 重新插拔设备。
- 在设备进入下载模式后再扫描端口。
- 安装或更新 Espressif USB/JTAG 驱动。

### 只看到 COM1

`COM1` 通常不是屏幕模块。请使用 `python -m serial.tools.list_ports -v` 查看包含 `303A:1001` 或 Espressif 信息的端口。

### 刷机卡在连接阶段

可以尝试：

- 按住设备 BOOT/下载键后执行刷机命令。
- 出现连接后松开 BOOT。
- 换 USB 线或 USB 口。
- 降低串口波特率，例如：

```powershell
idf.py -p COM3 -b 460800 flash
```

### 编译失败

优先检查：

- ESP-IDF 是否为 v5.1.4。
- 是否已加载 ESP-IDF 环境。
- 路径是否包含中文、空格或过长。
- 依赖组件是否完整。

可以先执行：

```powershell
idf.py fullclean
idf.py build
```

## 开发管理台

可编辑的新版网页源码在：

```text
web_new/
```

实际打包进固件的网页资源在：

```text
web/
main/include/webserver/
```

修改 `web_new` 后，需要同步到 `web` 并重新生成嵌入式头文件，再编译刷机。

常用流程：

```powershell
Copy-Item -Force web_new\index.html web\index.html
Copy-Item -Force web_new\index.css web\index.css
Copy-Item -Force web_new\index.js web\index.js
& "C:\Program Files\Git\bin\bash.exe" updateWeb.sh
idf.py build
idf.py -p COM3 flash
```

本地开发预览可能使用：

```text
http://127.0.0.1:5173/
```

这个地址仅用于电脑本机预览，不代表设备里的管理台。

## 验证命令

仓库中提供了一些轻量检查脚本，适合修改后自检：

```powershell
node --check web_new\index.js
node --check web\index.js
powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_web_cache_version.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_web_dirty_state.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pomodoro_pipeline.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_keytone_pipeline.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_webserver_reliability.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_wifi_pipeline.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_video_pipeline.ps1
idf.py build
```

发布或刷机前，至少建议执行 `idf.py build`，确保固件能完整编译。

## 目录结构

```text
main/                    ESP32-S3 固件源码
main/apps/               屏幕小应用
main/include/webserver/  自动生成的内置网页资源头文件
components/              Arduino、LVGL、TFT、音频和 LittleFS 等组件
web/                     会打进固件的网页资源
web_new/                 可编辑的新版管理台源码
tools/                   验证脚本
docs/images/             README 展示截图
partition_table.csv      分区表
sdkconfig                ESP-IDF 配置
```

## 固件限制和使用注意事项

- 屏幕分辨率是 `320 x 240`，高分辨率视频会被压缩和裁剪或补边。
- ESP32-S3 不是通用高清视频播放器，流畅度优先时应牺牲部分画质和声音。
- 视频带声音会增加负载，卡顿时请关闭视频声音并重新上传。
- 浏览器端转码需要电脑性能和网络加载转码器，首次转码可能较慢。
- 设备只支持 2.4GHz Wi-Fi，不支持 5GHz-only 网络。
- 路由器隔离、访客网络、公司/校园网络可能导致管理台无法访问。
- 刷机、擦除、分区变更可能导致设备内已有素材和设置丢失，请提前备份重要文件。
- 自定义音频文件不建议过大，过大的音频会占用 LittleFS 空间并影响上传体验。
- 本固件为社区修改版，刷机有风险，请确认设备型号匹配后再操作。

## 开源协议

本仓库代码以 0BSD 协议发布，详见 [LICENSE](LICENSE)。

第三方组件、字体、图片、音频、ESP-IDF、Arduino、LVGL、TFT_eSPI 等依赖仍遵循各自原始许可证。使用、分发或商用前，请自行确认相关第三方协议要求。
