#pragma once

#include "Arduino.h"
#include "WiFi.h"
#include <LittleFS.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"

#define PIN_DISPLAY_CS 10
#define PIN_DISPLAY_DC 11
#define PIN_DISPLAY_RST 7
#define PIN_DISPLAY_SCLK 9
#define PIN_DISPLAY_MOSI 8
#define PIN_DISPLAY_BL 14
#define PIN_DISPLAY_FSYNC 12

#define PIN_DISPLAY_PWR 13

#define PIN_RTC_SCLK 17
#define PIN_RTC_SDIO 18
#define PIN_RTC_RST 21

#define PIN_SERIAL2_RX 5
#define PIN_SERIAL2_TX 6

#define screenHeight 240
#define screenWidth 320
#define FIRMWARE_VERSION "SCM_V1.0.5"
#define FIRMWARE_VERSION_INT 10005
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
extern esp_lcd_panel_handle_t panel_handle;

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
