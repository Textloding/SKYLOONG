#pragma once

#include "Arduino.h"
#include "WiFi.h"
#include <DNSServer.h>
#include <LittleFS.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_check.h"

#define PIN_DISPLAY_BL 21
#define PIN_DISPLAY_PWR 3

#define PIN_RTC_SCLK 9
#define PIN_RTC_SDIO 11
#define PIN_RTC_RST 0

#define PIN_SERIAL1_RX 46
#define PIN_SERIAL1_TX 1

#define AUDIO_AMP_CTRL 10

#define AUDIO_I2S_GPIO_MCLK 7
#define AUDIO_I2S_GPIO_WS   6
#define AUDIO_I2S_GPIO_BCLK 2
#define AUDIO_I2S_GPIO_DIN  4
#define AUDIO_I2S_GPIO_DOUT 8

#define AUDIO_CODEC_I2C_SDA_PIN 12
#define AUDIO_CODEC_I2C_SCL_PIN 13

#define AUDIO_RECV_BUF_SIZE (2400)
#define AUDIO_SAMPLE_RATE (44100)
#define AUDIO_MCLK_MULTIPLE (256)
#define AUDIO_MCLK_FREQ_HZ (AUDIO_SAMPLE_RATE * AUDIO_MCLK_MULTIPLE)
#define AUDIO_VOICE_VOLUME (60)

#define screenHeight 240
#define screenWidth 320
#define FIRMWARE_VERSION "SCM_V4.0.0"
#define FIRMWARE_VERSION_INT 40000
#define WIFI_SAVE_MAX 50

#include "internationalization.h"
#include "hal.h"
#include "GUI.h"
#include "WiFiManager.h"
#include "AppManagerLite.h"
#include "APP_Def.hpp"

#include <driver/spi_common.h>
#include <driver/spi_master.h>
#include <esp_lcd_types.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_interface.h>
#include <esp_lcd_panel_commands.h>
#include <esp_lcd_panel_ops.h>

#include <TFT_eSPI.h>
extern TFT_eSPI tft;

extern bool RTC_DATA_ATTR screen_is_on;
extern bool RTC_DATA_ATTR screen_is_sleep;
/**
 * @brief 开始DPP配网并等待
 * @result
 *     0: 成功
 *    -1: 失败
 *    -2: 取消
 */
int esp_dpp_start(char *ssid, char *password);
void protocol_init(); // 初始化UART协议处理器

extern "C" const lv_font_t lv_font_chinese_16;
extern "C" const lv_font_t lv_font_big_32;
extern "C" const lv_font_t symbol_16;
#define SYMBOL_GLOBAL "\xEF\x82\xAC"
#define SYMBOL_24G "\xEF\x9A\x90"
#define SYMBOL_WIFI "\xEF\x87\xAB"
#define SYMBOL_BLUETOOTH "\xEF\x8A\x93"
#define SYMBOL_WINDOWS "\xEF\x85\xBA"
#define SYMBOL_APPLE "\xEF\x85\xB9"
#define SYMBOL_USB "\xEF\x8A\x87"
#define SYMBOL_CHARGING "\xEF\x83\xA7"
#define SYMBOL_BATTERY_100 "\xEF\x89\x80"
#define SYMBOL_BATTERY_75 "\xEF\x89\x81"
#define SYMBOL_BATTERY_50 "\xEF\x89\x82"
#define SYMBOL_BATTERY_25 "\xEF\x89\x83"
#define SYMBOL_BATTERY_0 "\xEF\x89\x84"
#define SYMBOL_COMMAND_LOCK "\xEE\x85\x82"
