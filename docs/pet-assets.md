# 键盘宠物素材说明

本固件的键盘宠物使用 OpenGameArt 上的开源像素狗素材：

- 素材名称：Dog 3
- 作者：rmazanek
- 来源页面：https://opengameart.org/content/dog-3
- 授权协议：CC0
- 原始文件：`assets/pet/dog_medium.png`

当前使用方式：

- `assets/pet/dog_medium.png` 保留原始 6 列 x 6 行 sprite sheet，便于追溯来源和后续重新生成资源。
- `main/pet_dog_sprites.c` 由 `tools/generate_pet_dog_sprites.ps1` 从原始 sprite sheet 生成，固件端直接使用 LVGL C 数组帧。
- 固件端帧会预先生成 2x 最近邻放大版本，输出尺寸为 120 x 76 px，每组动画使用 4 帧，避免 ESP32 屏幕端运行时用 `lv_img_set_zoom` 缩放导致闪烁或偶发不显示。
- `web/dog_medium.png` 和 `web_new/dog_medium.png` 用于管理台宠物预览，和屏幕端使用同一份视觉来源。

内置汪汪音效为本项目脚本合成的轻量 16 kHz mono PCM WAV，不使用第三方采样素材。

如果后续替换宠物素材，请优先选择 CC0、MIT、Apache-2.0 或明确允许商用和再分发的素材，并在本文档中补充来源、作者和授权说明。
