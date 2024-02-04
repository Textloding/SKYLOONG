#pragma once
#include "hal/ds1302.h"
#include <Preferences.h>
#include <Arduino.h>
enum system_event_type_t
{
    EVENT_GET_TIME = 0x01,
    EVENT_GOTO_SETTING,
    EVENT_EXIT_SETTING,
    EVENT_REQUIRE_SETTING,
    EVENT_TOGGLE_SCREEN_ON,
    EVENT_APM_CHANGED,
    EVENT_KB_STATUS_CHANGED,
    EVENT_SERVERCTL,
    EVENT_GOSLEEP,
};
typedef struct sysctl_event_t
{
    system_event_type_t type;
    uint8_t data;
} sysctl_event_t;
#define KB_CHANNEL_AVAILABLE_USB 0x01
#define KB_CHANNEL_AVAILABLE_24 0x02
#define KB_CHANNEL_AVAILABLE_BT1 0x04
#define KB_CHANNEL_AVAILABLE_BT2 0x08
typedef struct kb_status_t
{
    uint8_t channel_available : 4;
    uint8_t : 2;
    uint8_t channel_current : 2;
    uint8_t numlock : 1;
    uint8_t capslock : 1;
    uint8_t scrolllock : 1;
    uint8_t winlk : 1;
    uint8_t system : 1;
    uint8_t chan_state : 2;
    uint8_t : 1;
} kb_status_t;
enum battery_status_t : uint8_t
{
    BATTERY_STATUS_NORMAL = 0x00,
    BATTERY_STATUS_CHARGING = 0x01,
    BATTERY_STATUS_CHARGED = 0x02,
};
typedef struct kb_battery_t
{
    uint8_t value : 6; // 00H-32H: 0%-100%, LSB: 2%
    battery_status_t status : 2;
} kb_battery_t;

class HAL
{
public:
    DS1302_Dev rtc;
    DS1302_DateTime datetime;
    uint8_t battery_pct = 0;
    enum battery_status_t battery_status = BATTERY_STATUS_CHARGING;
    kb_status_t kb_status;
    bool setting_mode = false;              // 当前是否进入了设置模式
    bool lv_has_kb = false;                 // 是否当前显示了键盘控件，用于选择发送key的类型
    bool server_started = false;            // 是否已经启动了网页服务器
    bool config_show_battery_value = false; // 显示电池图标或数值
    bool config_time_12hr = false;
    SemaphoreHandle_t _mutex;
    void init();
    uint8_t getDoW(uint16_t iYear, uint8_t iMonth, uint8_t iDay);
    void getTime();
    void setTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
    bool NTPSync();
    void setBrightness(int8_t brightness);
    void LOCKLV();
    void UNLOCKLV();
    void requireSettings(bool req); // 要求用户进入设置模式
    bool getLastRequireSettings();  // 获取上一次是否要求过用户进入设置模式
    void goSleep();
    void send_sysctl(system_event_type_t type, uint8_t data = 0);
    void copy(File &newFile, File &file);
    void rm_rf(const char *path);
    void start_webserver();
    void stop_webserver();
    QueueHandle_t _queue;
    QueueHandle_t _queue_kb;
    Preferences pref;
    int8_t _brightness = 6;

private:
};
extern HAL hal;