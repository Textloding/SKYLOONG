#pragma once
#include "hal/ds1302.h"
#include <Preferences.h>
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "hal/i2s_types.h"
#include <atomic>
enum system_event_type_t
{
    EVENT_GET_TIME = 0x01,
    EVENT_GOTO_SETTING,
    EVENT_EXIT_SETTING,
    EVENT_REQUIRE_SETTING,
    EVENT_TOGGLE_SCREEN_ON,
    EVENT_APM_CHANGED,
    EVENT_KB_STATUS_CHANGED,
    EVENT_KB_KEYPRESS,
    EVENT_SERVERCTL,
    EVENT_GOSLEEP,
    EVENT_GOTO_QRCODE,
    EVENT_EXIT_QRCODE,
    EVENT_HOME_REFRESH,
    EVENT_GIF_REFRESH,
    EVENT_JPG_REFRESH,
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
    uint8_t channel_current : 2;            //0:24G, 1:BT1, 2:BT2, 3:USB
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

struct app_setting
{
    char weather_secret[64];
    char weather_city[64];
    char weather_provider[32];
    char weather_endpoint[128];
    char weather_endpoint_seniverse[128];
    char weather_endpoint_qweather[128];
    char weather_endpoint_aliyun_72158[128];
    char weather_endpoint_aliyun_10812[128];
    char weather_endpoint_aliyun_50139[128];
    char weather_endpoint_aliyun_71988[128];
    char weather_lat[24];
    char weather_lon[24];
    char weather_key_seniverse[64];
    char weather_key_qweather[64];
    char weather_key_aliyun_72158[64];
    char weather_key_aliyun_10812[64];
    char weather_key_aliyun_50139[64];
    char weather_key_aliyun_71988[64];
    char weather_appkey_aliyun_72158[64];
    char weather_appkey_aliyun_10812[64];
    char weather_appkey_aliyun_50139[64];
    char weather_appkey_aliyun_71988[64];
    char weather_appsecret_aliyun_72158[64];
    char weather_appsecret_aliyun_10812[64];
    char weather_appsecret_aliyun_50139[64];
    char weather_appsecret_aliyun_71988[64];
    char weather_publickey_seniverse[64];
};
extern struct app_setting app_settings_save;

class HAL
{
public:
    DS1302_Dev rtc;
    DS1302_DateTime datetime;
    uint8_t battery_pct = 0;
    enum battery_status_t battery_status = BATTERY_STATUS_CHARGING;
    kb_status_t kb_status;
    uint8_t APM = 0;
    bool APMChanged = false;
    bool setting_mode = false;              // 当前是否进入了设置模式
    bool qrcode_mode = false;               // 当前是否进入了二维码模式
    bool lv_has_kb = false;                 // 是否当前显示了键盘控件，用于选择发送key的类型
    bool server_started = false;            // 是否已经启动了网页服务器
    bool webserver_should_run = true;
    bool gif_update = false;
    bool jpg_update = false;
    bool weather_update = false;
    bool config_time_12hr = false;
    bool config_bootanimation = true;
    uint8_t config_theme = 0;
    uint8_t config_space_breath_speed = 1;
    uint8_t config_keytone = 0;
    char config_keytone_file[32];
    std::atomic_bool keytone_play{false};
    std::atomic_bool audio_ready{false};
    std::atomic<uint32_t> _keytone_request_generation{0};
    bool aps_enable = true;
    bool gif_enable = true;
    bool jpg_enable = true;
    bool weather_enable = true;
    uint32_t config_time_roll = 5000;
    bool lang_refresh = false;
    bool config_video_audio = false;
    char config_video_fit[16];
    char config_jpg_mode[32];
    char config_jpg_file[32];
    bool time_sync = true;
    SemaphoreHandle_t _mutex;
    SemaphoreHandle_t _rtc_mutex = NULL;
    SemaphoreHandle_t _audio_mutex = NULL;
    SemaphoreHandle_t _keytone_request_mutex = NULL;
    TimerHandle_t _audio_amp_idle_timer = NULL;
    TaskHandle_t _systemctl_task = NULL;
    std::atomic<TaskHandle_t> _audio_owner{NULL};
    std::atomic<uint32_t> _audio_stop_generation{0};
    std::atomic_bool _rtc_fallback_to_system{false};
    std::atomic_bool _audio_shutdown_requested{false};
    std::atomic_bool _audio_shutdown_complete{false};
    void init();
    esp_err_t audio_init();
    void audio_stop();
    bool audio_begin_playback();
    bool audio_configure_tx(uint32_t sample_rate, i2s_data_bit_width_t data_width, i2s_slot_mode_t slot_mode);
    void audio_end_playback();
    void audio_shutdown();
    uint8_t getDoW(uint16_t iYear, uint8_t iMonth, uint8_t iDay);
    bool refreshTime();
    void getTime();
    bool setTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
    bool NTPSync();
    void setBrightness(int8_t brightness);
    void setVolume(int8_t volume);
    void LOCKLV();
    void UNLOCKLV();
    void goSleep();
    void request_keytone();
    void send_sysctl(system_event_type_t type, uint8_t data = 0);
    void saveAppSettings();
    void loadAppSettings();
    void start_webserver();
    TaskHandle_t webserver_task;
    volatile uint32_t webserver_last_alive_ms = 0;
    void recover_webserver();
    void stop_webserver();
    void forceExitSettings();
    QueueHandle_t _queue;
    QueueHandle_t _queue_kb;
    QueueHandle_t _queue_keytone = NULL;
    Preferences pref;
    int8_t _brightness = 6;
    int8_t _volume = 6;

    char ssid[64];
    char password[64];
    bool config_wifi = false;

private:
    enum class audio_amp_mode_t : uint8_t
    {
        off,
        playback,
        idle_hold,
        shutdown,
    };

    portMUX_TYPE _audio_amp_mux = portMUX_INITIALIZER_UNLOCKED;
    portMUX_TYPE _audio_stop_mux = portMUX_INITIALIZER_UNLOCKED;
    audio_amp_mode_t _audio_amp_mode = audio_amp_mode_t::off;
    bool _audio_amp_gpio_high = false;
    TickType_t _audio_amp_idle_deadline = 0;
    uint32_t _audio_amp_epoch = 0;
    bool rtc_lock(TickType_t timeout);
    void rtc_unlock();
    bool audio_lock(TickType_t timeout);
    void audio_unlock();
    bool audio_publish_owner(TaskHandle_t owner, uint32_t stop_generation);
    void audio_clear_owner();
    void audio_stop_keytone(uint32_t generation);
    void audio_amp_set_level_locked(bool enabled);
    bool audio_amp_prepare_init();
    void audio_amp_mark_shutdown_complete(uint32_t epoch);
    bool audio_amp_shutdown_is_current(uint32_t epoch);
    bool audio_amp_start_playback(bool &cold_start);
    bool audio_amp_playback_ready();
    bool audio_amp_mute_for_reconfigure();
    bool audio_amp_resume_after_reconfigure();
    bool audio_amp_begin_idle_hold(uint32_t &epoch);
    void audio_amp_cancel_idle_hold(uint32_t epoch);
    void audio_amp_finish_idle_hold();
    uint32_t audio_amp_request_shutdown();
    void audio_amp_force_off();
    static void audio_amp_idle_timer_callback(TimerHandle_t timer);
    uint32_t _last_rtc_request_ms = 0;
};
extern HAL hal;
