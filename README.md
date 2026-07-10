# SKYLOONG ESP32-S3 屏幕固件

这是面向 SKYLOONG/GK87 系列 `320 x 240` 彩屏键盘屏幕模块的 ESP32-S3 固件。项目基于 ESP-IDF v5.1.4 与 Arduino 组件开发，主分支定位为基础稳定版，保留常用屏幕小应用、现代化网页管理台、Wi-Fi 配网与自动重连、素材上传、天气、视频/GIF 播放、音量与按键音设置。

键盘宠物、番茄钟、电脑监控不属于当前主分支功能，请不要按主分支 README 期待这些页面或设置项。

![管理台总览](docs/images/console-overview.png)

## 当前主分支包含什么

- 现代化网页管理台，支持电脑和手机浏览器访问。
- 屏幕小应用：主界面、手速监测、视频/GIF、图片相册、本地天气、设置、二维码/Wi-Fi 配网。
- 多个 Wi-Fi 凭据保存，启动后自动扫描并连接可用网络。
- 图片、MP4、GIF、MPEG 与自定义按键音上传。
- 浏览器端视频转码，面向 `320 x 240` 屏幕生成更适合 ESP32-S3 播放的 MPEG。
- 视频适配模式：完整显示或铺满屏幕。
- 设备音量调节、视频声音开关、内置按键音与自定义按键音。
- 天气城市、天气源、接口地址、API Key / AppCode / AppKey / AppSecret、经纬度配置。管理台会明文显示已保存的天气凭据，方便普通用户核对和修改。
- 屏幕主界面内置主题，包含新增太空人天气时钟主题。
- `/health`、`/info` 等接口用于排查联网和网页服务状态。

## 不包含什么

主分支已经精简空间，不包含以下实验功能：

- 键盘宠物。
- 番茄钟。
- 电脑监控页面或电脑端监控服务。

如果你需要实验功能，请切换到对应实验分支，并以该分支的说明为准。

## 管理台展示

管理台采用深色毛玻璃风格，刷入固件后由屏幕模块直接提供网页服务。开发时看到的 `http://127.0.0.1:5173/` 只是电脑本机预览地址，不是设备后台地址。

### 总览页

![总览页](docs/images/console-overview.png)

总览页展示设备联网状态、当前 IP、存储空间、素材数量、屏幕预览和常用开关。

### 媒体页

![媒体页](docs/images/console-media.png)

媒体页用于上传图片、视频/GIF 和音效文件。上传 MP4 或 GIF 时，浏览器会先转成适合小屏幕播放的 MPEG，再上传到设备。

视频策略：

- 屏幕物理分辨率：`320 x 240`。
- 默认输出：`320 x 240`、MPEG1 视频。
- 完整显示：保留完整画面，比例不一致时补边。
- 铺满屏幕：占满屏幕，比例不一致时裁切边缘。
- 播放视频声音：开启后尝试保留低码率单声道音轨；关闭后视频静音，更省资源。

如果视频卡顿，建议关闭视频声音、缩短视频时长，并重新上传原始视频或 GIF。

### 系统页

![系统页](docs/images/console-system.png)

系统页可设置设备音量、视频声音、按键音、天气城市、天气数据源、经纬度、时区、语言和设备重启。天气区域内置各天气源的 Key / AppCode 获取教程、接口地址说明和商品页入口，普通用户不需要翻 GitHub 文档也能完成配置。

## 适用设备

本固件面向使用 ESP32-S3、`320 x 240` 彩屏和 ES8311 音频芯片的 SKYLOONG/GK87 类屏幕模块。常见刷机串口特征如下：

```text
USB VID:PID = 303A:1001
设备名称可能显示为 USB Composite Device、USB JTAG/serial debug unit 或 USB 串行设备
串口示例：COM3
```

不同电脑上的端口号可能不同，刷机前必须以实际扫描结果为准。

## 推荐刷机方式

普通用户可以优先使用独立刷机工具：

```text
https://github.com/Textloding/skyloong-flasher
```

刷机工具用于选择本地固件包或仓库地址，并在界面里展示下载、解析、构建、串口扫描和刷机进度。若工具无法自动完成构建，或你正在开发固件，请使用下面的 ESP-IDF 命令方式。

## 手动构建与刷机

### 1. 准备硬件

- SKYLOONG/GK87 ESP32-S3 屏幕模块。
- 支持数据传输的 USB 线，不要使用只能充电的线。
- Windows 电脑更推荐，Linux/macOS 也可以，但命令需要按系统调整。

### 2. 准备软件

- Git。
- Python 3。
- ESP-IDF v5.1.4。
- ESP-IDF 对应工具链。

Windows 推荐使用 Espressif 官方安装器安装 ESP-IDF v5.1.4。安装后通常会有：

```text
C:\Users\<用户名>\esp\esp-idf
C:\Users\<用户名>\.espressif
```

### 3. 获取代码

```powershell
git clone https://github.com/Textloding/SKYLOONG.git
cd SKYLOONG
git switch main
```

### 4. 编译固件

打开 “ESP-IDF PowerShell” 或 “ESP-IDF Command Prompt”，进入仓库目录：

```powershell
cd C:\path\to\SKYLOONG
idf.py build
```

如果使用普通 PowerShell，需要先加载 ESP-IDF 环境：

```powershell
cd C:\path\to\SKYLOONG
. "$env:USERPROFILE\esp\esp-idf\export.ps1"
idf.py build
```

编译成功后，产物会生成在 `build` 目录。

### 5. 路径注意事项

ESP-IDF、CMake、Ninja 和部分 Python 工具对中文路径、空格路径、过长路径不够友好。遇到莫名编译错误时，建议把仓库复制到短英文路径，例如：

```powershell
C:\s
```

再执行：

```powershell
cd C:\s
idf.py build
```

### 6. 查看串口

连接屏幕后执行：

```powershell
python -m serial.tools.list_ports -v
```

找到类似 `VID:PID=303A:1001`、`USB JTAG/serial debug unit` 或 `USB Composite Device` 的端口，记下端口号，例如 `COM3`。

### 7. 刷入固件

把命令中的 `COM3` 换成实际端口：

```powershell
idf.py -p COM3 flash
```

刷入成功时通常会看到：

```text
Hash of data verified.
Leaving...
Hard resetting via RTS pin...
Done
```

刷机完成后设备会自动重启。

## 第一次进入管理台

### 设备已经保存过 Wi-Fi

设备启动后会自动扫描并连接已保存的 Wi-Fi。联网成功后，屏幕会显示当前局域网 IP。用电脑或手机访问：

```text
http://屏幕显示的IP/
```

例如：

```text
http://192.168.1.186/
```

### 设备还没有配置 Wi-Fi

设备会进入配网模式并开启热点：

```text
SKYLOONG 4.0 Screen
```

手机或电脑连接该热点后，打开：

```text
http://192.168.4.1/wifi
```

选择 2.4GHz Wi-Fi，输入密码并提交。提交后设备会连接新网络，请等待屏幕显示新的局域网 IP，再用新 IP 打开管理台。

### 127.0.0.1 不是设备后台

开发时可能会看到：

```text
http://127.0.0.1:5173/
```

这个地址只表示电脑本机的开发预览服务。刷进屏幕后，普通用户应访问屏幕显示的局域网 IP。

## 日常使用

### 切换屏幕页面

屏幕页面切换沿用键盘固件提供的组合键，常见为：

```text
fn + ~
```

具体组合键可能随键盘型号或键盘固件变化，请以你的键盘说明为准。

### 图片

进入管理台“媒体”页上传图片。管理台会按屏幕比例预览，适合用作屏幕图片或相册内容。

### 视频或 GIF

进入“媒体”页选择 MP4、GIF 或 MPEG。MP4/GIF 会在浏览器内转码为小尺寸 MPEG 后上传。

建议：

- 优先使用短视频片段。
- 源文件可以是高清或 4K，但最终会压缩到 `320 x 240`。
- 想要更流畅时，关闭“播放视频声音”并重新上传。
- 想铺满屏幕时选择“铺满屏幕”，这可能裁掉边缘。
- 想保留完整画面时选择“完整显示”，这可能出现补边。

### 声音和音量

屏幕模块带音频输出能力。主分支支持：

- 系统页调节设备音量。
- 设置视频是否播放声音。
- 上传并试听自定义按键音。
- 使用内置按键音：内置 1、内置 2、内置 3、弹跳三连、篮球律动、趣味上扬。

视频声音会占用更多解码和播放资源。若视频画面或声音卡顿，请关闭视频声音并重新上传视频。

### 天气

系统页支持设置天气城市、天气源、接口地址、API Key / AppCode / AppKey / AppSecret、纬度和经度。每个天气源会独立记住自己的 Key、阿里云三项凭据和接口地址；切换到其他源再切回来时，会带回该源已经保存过的配置。

当前管理台提供：

- 心知天气：需要用户自己的 API Key。
- QWeather 和风天气：需要用户自己的 API Key。
- 阿里云市场 72158 精准天气：需要在阿里云云市场开通商品并复制 AppCode，或复制 AppKey 和 AppSecret 使用签名鉴权。
- 阿里云市场 10812 万维易源：需要在阿里云云市场开通商品并复制 AppCode，或复制 AppKey 和 AppSecret 使用签名鉴权。
- 阿里云市场 50139 实时多天天气：需要在阿里云云市场开通商品并复制 AppCode，或复制 AppKey 和 AppSecret 使用签名鉴权。
- 阿里云市场 71988 快证天气：需要在阿里云云市场开通商品并复制 AppCode，或复制 AppKey 和 AppSecret 使用签名鉴权。

#### 心知天气

1. 打开心知天气文档：<https://docs.seniverse.com/api/weather/now.html>。
2. 注册并登录心知天气控制台，创建免费或付费产品。
3. 复制产品里的私钥 Key。
4. 管理台选择“心知天气”，`API Key` 填私钥 Key。
5. `接口地址` 默认填 `http://api.seniverse.com`。固件会自动拼接 `/v3/weather/now.json`、`/v3/weather/daily.json` 等路径，普通用户不需要改。

#### QWeather 和风天气

1. 打开 QWeather 认证文档：<https://dev.qweather.com/docs/configuration/authentication/>。
2. 登录 QWeather 开发平台，创建项目并添加 Web API Key。
3. 复制 API KEY。
4. 管理台选择“QWeather 和风天气”，`API Key` 填 API KEY。
5. `接口地址` 默认填 `https://devapi.qweather.com`。固件会自动访问 `/v7/weather/now`、`/v7/weather/3d`、`/v7/indices/1d`。

#### 阿里云云市场天气源通用步骤

阿里云源不是填阿里云账号密码，也不是填 AccessKey。这里填对应商品的 `AppCode`、`AppKey`、`AppSecret` 三项凭据：优先推荐 AppCode；如果你只想用签名鉴权，可以留空 AppCode，同时填写 AppKey 和 AppSecret。

1. 打开对应商品页，登录阿里云。
2. 开通免费规格或购买套餐。
3. 进入阿里云云市场控制台，找到“已购买的服务”或该商品的调用凭证，复制 `AppCode`、`AppKey`、`AppSecret`。
4. 回到商品页，打开“API接口”“接口文档”或“API调试”区域。
5. 复制该接口的 `Host + Path` 完整请求地址，填入管理台的 `接口地址`。
6. 管理台的 `AppCode` 输入框只填 AppCode 本身，不要手动加 `APPCODE` 前缀；`AppKey` 和 `AppSecret` 分别填入对应字段。
7. 保存后，固件会优先使用 `Authorization: APPCODE <你的 AppCode>` 请求头；AppCode 为空时，会用 AppKey/AppSecret 生成 `X-Ca-Key`、`X-Ca-Signature` 等阿里云网关签名请求头。

当前内置的阿里云商品入口和默认接口地址如下：

| 天气源 | 商品页 | 默认接口地址 |
| --- | --- | --- |
| 阿里云 72158 精准天气 | <https://market.aliyun.com/detail/cmapi00072158.html> | `https://getweather.market.alicloudapi.com/lundear/weather1d` |
| 阿里云 10812 万维易源 | <https://market.aliyun.com/detail/cmapi010812.html> | `https://ali-weather.showapi.com/spot-to-weather` |
| 阿里云 50139 实时多天天气 | <https://market.aliyun.com/detail/cmapi00050139.html> | `https://weather01.market.alicloudapi.com/weather` |
| 阿里云 71988 快证天气 | <https://market.aliyun.com/detail/cmapi00071988.html> | `https://kzweather.market.alicloudapi.com/weather` |

注意：阿里云云市场不同厂商的接口地址、参数名、返回字段可能会调整。管理台允许手动修改接口地址；如果商品页“API接口/调试”里展示的地址和上表默认值不同，请以商品页为准。每个天气源的接口地址和三项凭据会独立保存，不会再因为切换天气源而互相覆盖。当前主分支已弃用 Open-Meteo，因为它在国内网络下不稳定且可能返回参数错误。

自动识别城市依赖浏览器定位或网络 IP 定位，可能受浏览器权限、代理、网络环境和第三方定位服务影响。不准确时请手动填写城市和经纬度。

## 联网和访问注意事项

### 启动后需要等待

设备上电或刷机后，需要完成启动、加载小应用、连接 Wi-Fi、启动网页服务。这个过程可能需要 10 到 60 秒。

如果刚开机访问管理台很慢、打不开或只看到背景，请先等待半分钟到一分钟，再刷新页面。

### 切换 Wi-Fi 后 IP 可能变化

在管理台提交新的 Wi-Fi 后，设备会断开旧网络并连接新网络。原网页地址可能失效，请看屏幕显示的新 IP，再重新打开。

### 必须在同一局域网

电脑或手机必须和屏幕处于同一局域网，才能通过 `http://屏幕IP/` 访问管理台。

常见问题：

- 手机使用蜂窝网络时访问不到屏幕。
- 电脑和屏幕连接到不同网络。
- 路由器开启 AP 隔离或访客网络隔离。
- 公司、学校、酒店网络禁止设备互访。

### 只剩背景或加载很慢

按顺序排查：

1. 等待 30 到 60 秒后刷新。
2. 确认访问的是屏幕 IP，不是 `127.0.0.1:5173`。
3. 确认电脑和屏幕在同一网络。
4. 浏览器强制刷新，或在地址后加版本参数，例如 `http://屏幕IP/?v=latest`。
5. 在管理台可进入时点击“重启设备”，或断电重插。
6. 长期运行后仍频繁异常时，通过串口查看日志并提交 issue。

### 健康检查接口

能访问设备 IP 但页面异常时，可直接打开：

```text
http://屏幕IP/health
http://屏幕IP/info
```

`/health` 用于查看网页服务、Wi-Fi、剩余堆内存等状态。`/info` 用于查看当前配置状态。

## 常见刷机问题

### 找不到串口

- 换一根支持数据传输的 USB 线。
- 换电脑 USB 口。
- 重新插拔设备。
- 安装或更新 Espressif USB/JTAG 驱动。
- 使用 `python -m serial.tools.list_ports -v` 查看详细设备信息。

### 只看到 COM1

`COM1` 通常不是屏幕模块。请寻找包含 `303A:1001` 或 Espressif 信息的端口。

### 刷机卡在连接阶段

可以尝试：

- 按住设备 BOOT/下载键后执行刷机命令。
- 出现连接后松开 BOOT。
- 换 USB 线或 USB 口。
- 降低串口波特率：

```powershell
idf.py -p COM3 -b 460800 flash
```

### 编译失败

优先检查：

- ESP-IDF 是否为 v5.1.4。
- 是否已经加载 ESP-IDF 环境。
- 路径是否包含中文、空格或过长。
- 依赖组件是否完整。

可尝试：

```powershell
idf.py fullclean
idf.py build
```

## 开发管理台

可编辑的管理台源码在：

```text
web_new/
```

实际打包进固件的网页资源在：

```text
web/
main/include/webserver/
```

修改 `web_new` 后，需要同步到 `web`，再重新生成内置网页资源头文件，然后编译刷机：

```powershell
Copy-Item -Force web_new\index.html web\index.html
Copy-Item -Force web_new\index.css web\index.css
Copy-Item -Force web_new\index.js web\index.js
& "C:\Program Files\Git\bin\bash.exe" updateWeb.sh
idf.py build
idf.py -p COM3 flash
```

本地开发预览地址通常是：

```text
http://127.0.0.1:5173/
```

它只用于电脑本机预览，不代表设备里的管理台。

## 验证命令

修改后建议至少执行：

```powershell
node --check web_new\index.js
node --check web\index.js
powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_web_cache_version.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_keytone_pipeline.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_video_pipeline.ps1
idf.py build
```

如果只改了 README，可不执行固件编译；如果改了 `main/`、`components/` 或内置网页资源，发布或刷机前建议执行 `idf.py build`。

## 目录结构

```text
main/                    ESP32-S3 固件源码
main/apps/               屏幕小应用
main/img_space/          太空人主题素材与说明
main/include/webserver/  自动生成的内置网页资源头文件
components/              Arduino、LVGL、TFT、音频和 LittleFS 等组件
web/                     打包进固件的网页资源
web_new/                 可编辑的管理台源码
tools/                   轻量验证脚本
docs/images/             README 展示截图
partition_table.csv      分区表
sdkconfig                ESP-IDF 配置
```

## 固件限制和注意事项

- 屏幕分辨率是 `320 x 240`，高分辨率视频会被压缩、裁切或补边。
- ESP32-S3 不是通用高清视频播放器，追求流畅时需要牺牲部分画质和声音。
- 视频带声音会增加负载，卡顿时请关闭视频声音并重新上传。
- 浏览器端转码依赖电脑性能，首次加载转码器可能较慢。
- 设备只支持 2.4GHz Wi-Fi，不支持 5GHz-only 网络。
- 路由器隔离、访客网络、公司或校园网络可能导致管理台无法访问。
- 刷机、擦除、分区变更可能导致设备内已有素材和设置丢失，请提前备份重要文件。
- 自定义音频文件不建议过大，过大的音频会占用 LittleFS 空间并影响上传体验。
- 本固件为社区修改版，刷机有风险，请确认设备型号匹配后再操作。

## 开源协议

本仓库代码以 0BSD 协议发布，详见 [LICENSE](LICENSE)。

第三方组件、字体、图片、音频、ESP-IDF、Arduino、LVGL、TFT_eSPI 等依赖仍遵循各自原始许可证。使用、分发或商用前，请自行确认相关第三方许可要求。
