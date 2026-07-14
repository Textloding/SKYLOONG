#include "A_Config.h"

#include "esp_heap_caps.h"
#include "Wire.h"
#include "es8311.h"
#include "ESP_I2S.h"
#include "keytone/keytone1.h"
#include "keytone/keytone2.h"
#include "keytone/keytone3.h"
#include "keytone/keytone4.h"
#include "keytone/keytone5.h"
#include "keytone/keytone6.h"

I2SClass i2s;

es8311_handle_t es_handle = es8311_create(I2C_NUM_0, ES8311_ADDRRES_0);
static const char *TAG = "audio_init";
static constexpr uint32_t AUDIO_DMA_TAIL_GUARD_MS = 50;
static constexpr uint32_t RTC_READ_INTERVAL_MS = 500;
extern volatile bool gif_vid_stop;

#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI(); 

HAL hal;
#define DRAW_BUF_SIZE (2 * screenWidth * screenHeight)
lv_indev_t *indev_keypad;

static int first_refresh = 3;

void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h);
    tft.endWrite();

    if (first_refresh > 0)
    {
        first_refresh--;
        if (first_refresh == 0)
        {
            if(screen_is_on == true) {
                screen_is_sleep = false;
                hal.setBrightness(hal._brightness);
                audio_is_sleep = false;
                hal.setVolume(hal._volume);
            }
            hal.time_sync = true;
            ESP_LOGW("HAL", "首次刷新完成");
            first_refresh = -1;
        }
    }
    lv_disp_flush_ready(disp_drv);
}

const char* getFileSuffix(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

bool hasFileSuffix(const char *filename, const char *suffix) {
    const char *file_suffix = getFileSuffix(filename);
    return strcmp(file_suffix, suffix) == 0;
}
size_t sizeFile(String path) {
  File file = LittleFS.open(path, "r", false); 
  size_t size = file.size();
  file.close();
  return size;
}

uint8_t * readFile(String path) {
  File file = LittleFS.open(path, "r", false); 
  uint8_t *buf = (uint8_t *)malloc(file.size());
  if (buf == NULL) {
    file.close();
    return NULL;
  }

  int bytesRead = file.readBytes((char *)buf, file.size());
  if (bytesRead != file.size()) {
    file.close();
    return NULL;
  }

  file.close();
  return buf;
}

static bool playWavWithAmplifier(const uint8_t *data, size_t len)
{
    if (data == NULL || len == 0 || !hal.audio_begin_playback())
        return false;

    i2s.playWAV(data, len);
    hal.audio_end_playback();
    return true;
}

static bool playAudioFileFromLittleFS(const char *filename)
{
    if (!hal.audio_ready || filename == NULL || strlen(filename) == 0)
        return false;

    char path[64] = "/";
    strncat(path, filename, sizeof(path) - strlen(path) - 1);

    uint8_t *data = readFile(path);
    if (data == NULL)
        return false;

    const size_t len = sizeFile(path);
    bool played = false;
    const bool is_wav = hasFileSuffix(filename, "wav");
    const bool is_mp3 = hasFileSuffix(filename, "mp3");
    if ((is_wav || is_mp3) && hal.audio_begin_playback())
    {
        if (is_wav)
        {
            i2s.playWAV(data, len);
            played = true;
        }
        else
        {
            played = i2s.playMP3(data, len);
        }
        hal.audio_end_playback();
    }

    free(data);
    return played;
}

static void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static uint8_t last_key = 0xff;
    static uint8_t act_key = 0;
    static bool last_pressed = false;
    data->state = LV_INDEV_STATE_REL;
    data->key = last_key;
    if (last_pressed)
    {
        last_pressed = false;
        return;
    }
    if (xQueueReceive(hal._queue_kb, &act_key, 0) != pdTRUE)
        return;
    data->key = act_key;
    last_pressed = true;
    data->state = LV_INDEV_STATE_PR;
    last_key = act_key;
}

static void task_systemctl(void *p)
{
    bool rtc_was_halted = false;
    if (hal._rtc_mutex != NULL && xSemaphoreTake(hal._rtc_mutex, pdMS_TO_TICKS(500)) == pdTRUE)
    {
        rtc_was_halted = DS1302_isHalted(&hal.rtc);
        if (rtc_was_halted)
        {
            DS1302_writeProtect(&hal.rtc, false);
            DS1302_halt(&hal.rtc, false);
        }
        xSemaphoreGive(hal._rtc_mutex);
    }
    if (rtc_was_halted)
        GUI::toast(_tr(I18N_ID_RTC_SHUTDOWN), true, 10000);
    while (1)
    {
        sysctl_event_t event;
        xQueueReceive(hal._queue, &event, portMAX_DELAY);
        switch (event.type)
        {
        case EVENT_GET_TIME:
            hal.refreshTime();
            break;
        case EVENT_GOTO_SETTING:
            hal.setting_mode = true;
            appManagerLite.switchSetting();
            break;
        case EVENT_EXIT_SETTING:
            hal.setting_mode = false;
            appManagerLite.exitSetting();
            break;
        case EVENT_REQUIRE_SETTING:
            break;
        case EVENT_TOGGLE_SCREEN_ON:
            if (event.data == 0)
            {
                ledcWrite(PIN_DISPLAY_BL, 0);
                hal.audio_shutdown();
            }
            else
            {
                hal.setBrightness(hal._brightness);
                hal.audio_init();
                hal.setVolume(hal._volume);
            }
            break;
        case EVENT_APM_CHANGED:
            hal.APM = event.data;
            hal.APMChanged = true;
            break;
        case EVENT_KB_KEYPRESS:
            if (!hal.audio_ready)
                break;
            if (hal.config_keytone == 1) {
                hal.keytone_play =  true;
                playWavWithAmplifier(__keytone_keytone1_wav, __keytone_keytone1_wav_len);
                hal.keytone_play =  false;
            } else if (hal.config_keytone == 2) {
                hal.keytone_play =  true;
                playWavWithAmplifier(__keytone_keytone2_wav, __keytone_keytone2_wav_len);
                hal.keytone_play =  false;
            } else if (hal.config_keytone == 3) {
                hal.keytone_play =  true;
                playWavWithAmplifier(__keytone_keytone3_wav, __keytone_keytone3_wav_len);
                hal.keytone_play =  false;
            } else if (hal.config_keytone == 4) {
                hal.keytone_play =  true;
                playAudioFileFromLittleFS(hal.config_keytone_file);
                hal.keytone_play =  false;
            } else if (hal.config_keytone == 5) {
                hal.keytone_play =  true;
                playWavWithAmplifier(__keytone_keytone4_wav, __keytone_keytone4_wav_len);
                hal.keytone_play =  false;
            } else if (hal.config_keytone == 6) {
                hal.keytone_play =  true;
                playWavWithAmplifier(__keytone_keytone5_wav, __keytone_keytone5_wav_len);
                hal.keytone_play =  false;
            } else if (hal.config_keytone == 7) {
                hal.keytone_play =  true;
                playWavWithAmplifier(__keytone_keytone6_wav, __keytone_keytone6_wav_len);
                hal.keytone_play =  false;
            }
            break;
        case EVENT_SERVERCTL:
            if (event.data == 1)
                hal.start_webserver();
            else
                hal.stop_webserver();
            break;
        case EVENT_GOSLEEP:
        {
            extern volatile bool gif_vid_stop;
            gif_vid_stop = true;
            if (xSemaphoreTake(hal._mutex, 4000) == pdTRUE)
            {
                if (hal.server_started == false)
                {
                    hal.audio_shutdown();
                    hal.goSleep();
                    hal.audio_init();
                    hal.setVolume(hal._volume);   
                }
                else
                {
                    screen_is_sleep = true;
                    ledcWrite(PIN_DISPLAY_BL, 0);
                    audio_is_sleep = true;
                    hal.audio_shutdown();
                    xSemaphoreGive(hal._mutex);
                }
            }
            else
            {
                ESP_LOGE("HAL", "无法获取锁");
                hal.audio_shutdown();
                hal.goSleep();
                hal.audio_init();
                hal.setVolume(hal._volume);
            }
        }
        break;
        case EVENT_GOTO_QRCODE:
            hal.qrcode_mode = true;
            appManagerLite.switchQRCode();
            break;
        case EVENT_EXIT_QRCODE:
            hal.qrcode_mode = false;
            appManagerLite.exitQRCode();
            break;
        case EVENT_HOME_REFRESH:
            appManagerLite.switchApp(1);
            break;
        case EVENT_GIF_REFRESH:
            appManagerLite.switchApp(3);
            break;
        case EVENT_JPG_REFRESH:
            appManagerLite.switchApp(4);
            break;
        default:
            break;
        }
    }
}

void lcd_init()
{
    pinMode(PIN_DISPLAY_PWR, OUTPUT);
    digitalWrite(PIN_DISPLAY_PWR, LOW);
    pinMode(PIN_DISPLAY_BL, OUTPUT);
    ledcAttach(PIN_DISPLAY_BL, 16000, 8);
    ledcWrite(PIN_DISPLAY_BL, 0);

    delay(1000);

    tft.init();

    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.invertDisplay(true);
}

static bool isVideoFitValueValid(const String &video_fit)
{
    return video_fit == "contain" || video_fit == "cover";
}

static void setVideoFitConfig(const String &video_fit)
{
    const String safe_video_fit = isVideoFitValueValid(video_fit) ? video_fit : "contain";
    strncpy(hal.config_video_fit, safe_video_fit.c_str(), sizeof(hal.config_video_fit) - 1);
    hal.config_video_fit[sizeof(hal.config_video_fit) - 1] = '\0';
}

bool HAL::audio_lock(TickType_t timeout)
{
    if (_audio_mutex == NULL)
        return false;
    return xSemaphoreTakeRecursive(_audio_mutex, timeout) == pdTRUE;
}

void HAL::audio_unlock()
{
    if (_audio_mutex != NULL)
        xSemaphoreGiveRecursive(_audio_mutex);
}

esp_err_t HAL::audio_init()
{
    if (!audio_lock(portMAX_DELAY))
    {
        ESP_LOGE(TAG, "Audio mutex is unavailable");
        return ESP_FAIL;
    }

    if (audio_ready && !_audio_shutdown_requested.load())
    {
        audio_unlock();
        return ESP_OK;
    }

    pinMode(AUDIO_AMP_CTRL, OUTPUT);
    digitalWrite(AUDIO_AMP_CTRL, LOW);

    if (_audio_shutdown_requested.exchange(false))
    {
        audio_ready = false;
        if (i2s.txChan() != NULL || i2s.rxChan() != NULL)
            i2s.end();
        if (es_handle != NULL)
        {
            es8311_voice_volume_set(es_handle, 0, NULL);
            es8311_power_down(es_handle);
        }
    }
    audio_ready = false;

    if (!Wire.begin(AUDIO_CODEC_I2C_SDA_PIN, AUDIO_CODEC_I2C_SCL_PIN))
    {
        ESP_LOGE(TAG, "I2C initialization failed");
        audio_unlock();
        return ESP_FAIL;
    }

    if (es_handle == NULL)
    {
        ESP_LOGE(TAG, "es8311 create failed");
        audio_unlock();
        return ESP_FAIL;
    }

    const es8311_clock_config_t es_clk = {
        .mclk_inverted = false,
        .sclk_inverted = false,
        .mclk_from_mclk_pin = true,
        .mclk_frequency = AUDIO_MCLK_FREQ_HZ,
        .sample_frequency = AUDIO_SAMPLE_RATE
    };
    esp_err_t codec_err = ESP_FAIL;
    for (uint8_t attempt = 1; attempt <= 3; ++attempt)
    {
        codec_err = es8311_init(es_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16);
        if (codec_err == ESP_OK)
            break;
        ESP_LOGW(TAG, "ES8311 init attempt %u failed: %s (0x%x)",
                 (unsigned)attempt, esp_err_to_name(codec_err), (unsigned)codec_err);
        delay(50);
    }
    if (codec_err != ESP_OK)
    {
        ESP_LOGE(TAG, "ES8311 unavailable; continuing with audio disabled");
        audio_unlock();
        return codec_err;
    }

    codec_err = es8311_microphone_config(es_handle, false);
    if (codec_err != ESP_OK)
    {
        ESP_LOGE(TAG, "ES8311 microphone setup failed: %s (0x%x)",
                 esp_err_to_name(codec_err), (unsigned)codec_err);
        audio_unlock();
        return codec_err;
    }

    i2s.setPins(AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN, AUDIO_I2S_GPIO_MCLK);
    if (!i2s.begin(I2S_MODE_STD, AUDIO_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH))
    {
        ESP_LOGE(TAG, "I2S initialization failed");
        audio_unlock();
        return ESP_FAIL;
    }

    codec_err = es8311_voice_volume_set(es_handle, 0, NULL);
    if (codec_err != ESP_OK)
    {
        ESP_LOGE(TAG, "ES8311 mute failed: %s (0x%x)",
                 esp_err_to_name(codec_err), (unsigned)codec_err);
        if (i2s.txChan() != NULL || i2s.rxChan() != NULL)
            i2s.end();
        audio_unlock();
        return codec_err;
    }

    audio_ready = true;
    ESP_LOGI(TAG, "Audio codec and I2S are ready; amplifier remains muted");
    audio_unlock();
    return ESP_OK;
}

void HAL::audio_stop()
{
    const TaskHandle_t current = xTaskGetCurrentTaskHandle();
    if (_audio_owner != NULL && _audio_owner != current)
    {
        // I2SClass::stop only sets its cancellation flag. Use it as an
        // asynchronous stop request while the playback owner drains/cleans up.
        i2s.stop();
    }

    if (!audio_lock(pdMS_TO_TICKS(2000)))
        return;

    if (audio_ready)
        i2s.stop();
    if (_audio_owner == current)
        audio_end_playback();
    audio_unlock();
}

bool HAL::audio_begin_playback()
{
    if (!audio_lock(portMAX_DELAY))
        return false;

    if (_audio_shutdown_requested.load())
    {
        audio_unlock();
        return false;
    }

    const TaskHandle_t current = xTaskGetCurrentTaskHandle();
    if (_audio_owner != NULL)
    {
        ESP_LOGW(TAG, "Audio playback already owned by another playback scope");
        audio_unlock();
        return false;
    }

    pinMode(AUDIO_AMP_CTRL, OUTPUT);
    digitalWrite(AUDIO_AMP_CTRL, LOW);
    if (!audio_ready || _volume <= 0)
    {
        audio_unlock();
        return false;
    }

    setVolume(_volume);
    if (!audio_ready)
    {
        audio_unlock();
        return false;
    }

    i2s.clearStop();
    _audio_owner = current;
    digitalWrite(AUDIO_AMP_CTRL, HIGH);
    return true;
}

void HAL::audio_end_playback()
{
    const TaskHandle_t current = xTaskGetCurrentTaskHandle();
    if (_audio_owner != current)
    {
        if (_audio_owner != NULL)
            ESP_LOGW(TAG, "Ignoring audio_end_playback from non-owner task");
        return;
    }

    // i2s_channel_write() returns after copying into DMA. Keep the amplifier
    // enabled until the final DMA buffers have physically drained.
    if (i2s.txChan() != NULL)
        delay(AUDIO_DMA_TAIL_GUARD_MS);
    pinMode(AUDIO_AMP_CTRL, OUTPUT);
    digitalWrite(AUDIO_AMP_CTRL, LOW);
    _audio_owner = NULL;
    audio_unlock();
}

void HAL::audio_shutdown()
{
    _audio_shutdown_requested.store(true);
    i2s.stop();
    gif_vid_stop = true;
    if (!audio_lock(pdMS_TO_TICKS(3000)))
    {
        pinMode(AUDIO_AMP_CTRL, OUTPUT);
        digitalWrite(AUDIO_AMP_CTRL, LOW);
        ESP_LOGE(TAG, "Timed out waiting for active playback to stop");
        return;
    }

    const TaskHandle_t current = xTaskGetCurrentTaskHandle();
    if (_audio_owner == current)
    {
        audio_end_playback();
    }

    pinMode(AUDIO_AMP_CTRL, OUTPUT);
    digitalWrite(AUDIO_AMP_CTRL, LOW);
    audio_ready = false;

    if (i2s.txChan() != NULL || i2s.rxChan() != NULL)
        i2s.end();
    if (es_handle != NULL)
    {
        es8311_voice_volume_set(es_handle, 0, NULL);
        es8311_power_down(es_handle);
    }
    audio_unlock();
}

void HAL::init()
{
    memset(&datetime, 0, sizeof(DS1302_DateTime));
    WiFi.setHostname("SKYLOONG 4.0 Screen");
    pinMode(AUDIO_AMP_CTRL, OUTPUT);
    digitalWrite(AUDIO_AMP_CTRL, LOW);
    lcd_init();

    if (_audio_mutex == NULL)
        _audio_mutex = xSemaphoreCreateRecursiveMutex();
    if (_audio_mutex == NULL)
    {
        ESP_LOGE("HAL", "Audio mutex allocation failed; audio disabled");
    }
    else
    {
        const esp_err_t audio_err = hal.audio_init();
        if (audio_err != ESP_OK)
            ESP_LOGE("HAL", "Audio disabled after initialization error: %s (0x%x)",
                     esp_err_to_name(audio_err), (unsigned)audio_err);
    }

    if (_rtc_mutex == NULL)
        _rtc_mutex = xSemaphoreCreateMutex();
    if (_rtc_mutex == NULL)
        ESP_LOGE("HAL", "RTC mutex allocation failed");

    DS1302_begin(&rtc, PIN_RTC_SCLK, PIN_RTC_SDIO, PIN_RTC_RST);
    DS1302_writeClockRegister(&rtc, DS1302_REG_TC, 0xA5);
    if (LittleFS.begin(false) == false)
    {
        LittleFS.format();
        if (LittleFS.begin(false) == false)
        {
            ESP_LOGE("HAL", "LittleFS格式化失败");
            while (1)
                vTaskDelay(1000);
        }
    }

    loadAppSettings();
    pref.begin("settings", false);
    hal._brightness = pref.getUInt("bright", 6);
    hal._volume = pref.getUInt("volume", 6);
    hal.config_time_12hr = pref.getBool("12hr", false);
    hal.config_bootanimation = pref.getBool("s_b_a", true);
    hal.config_theme = pref.getInt("theme", 0);
    hal.config_space_breath_speed = pref.getUInt("space_breath", 1);
    if (hal.config_space_breath_speed > 2)
        hal.config_space_breath_speed = 1;
    hal.config_keytone = pref.getInt("keytone", 0);
    strcpy(hal.config_keytone_file, pref.getString("keytone_file", "").c_str());
    hal.aps_enable = pref.getBool("aps_enable", true);
    hal.weather_enable = pref.getBool("weather_enable", true);
    hal.gif_enable = pref.getBool("gif_enable", true);
    hal.jpg_enable = pref.getBool("jpg_enable", true);
    hal.config_time_roll = pref.getInt("t_r", 5000);
    hal.config_video_audio = pref.getBool("video_audio", false);
    setVideoFitConfig(pref.getString("video_fit", "contain"));
    strcpy(hal.config_jpg_mode, pref.getString("jpg_mode", "roll").c_str());
    strcpy(hal.config_jpg_file, pref.getString("jpg_file", "").c_str());
    i18n::setLanguage(pref.getUInt("lang", 0));
    i18n::setNTPOffset(pref.getInt("ntp", 3600 * 8));
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t *screen_buf = NULL;
    static lv_color_t *screen_buf2 = NULL;
    static lv_disp_drv_t disp_drv;
    static lv_indev_drv_t indev_drv;

    _mutex = xSemaphoreCreateMutex();
    _queue = xQueueCreate(10, sizeof(sysctl_event_t));
    _queue_kb = xQueueCreate(10, 1);
    lv_init();

    uint32_t draw_buf_pixels = DRAW_BUF_SIZE / sizeof(lv_color_t);
    screen_buf = (lv_color_t *)heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    screen_buf2 = (lv_color_t *)heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    if (screen_buf == NULL && screen_buf2 != NULL)
    {
        screen_buf = screen_buf2;
        screen_buf2 = NULL;
    }
    if (screen_buf == NULL)
    {
        draw_buf_pixels = screenWidth * 20;
        screen_buf = (lv_color_t *)heap_caps_malloc(draw_buf_pixels * sizeof(lv_color_t), MALLOC_CAP_8BIT);
        screen_buf2 = NULL;
        ESP_LOGW("HAL", "PSRAM draw buffers unavailable; using a %" PRIu32 "-pixel fallback", draw_buf_pixels);
    }
    if (screen_buf == NULL)
    {
        ESP_LOGE("HAL", "LVGL draw-buffer allocation failed; internal=%" PRIu32 ", psram=%" PRIu32,
                 (uint32_t)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                 (uint32_t)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        while (true)
            vTaskDelay(pdMS_TO_TICKS(1000));
    }

    lv_disp_draw_buf_init(&draw_buf, screen_buf, screen_buf2, draw_buf_pixels);
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = keypad_read;
    indev_keypad = lv_indev_drv_register(&indev_drv);

    lv_group_t *group = lv_group_create();
    lv_group_set_default(group);
    lv_indev_set_group(indev_keypad, group);

    if (xTaskCreatePinnedToCore(task_systemctl, "task_systemctl", 8192, NULL, 3, &_systemctl_task, 0) != pdPASS)
    {
        _systemctl_task = NULL;
        ESP_LOGE("HAL", "Failed to create task_systemctl");
    }
}

bool HAL::rtc_lock(TickType_t timeout)
{
    return _rtc_mutex != NULL && xSemaphoreTake(_rtc_mutex, timeout) == pdTRUE;
}

void HAL::rtc_unlock()
{
    if (_rtc_mutex != NULL)
        xSemaphoreGive(_rtc_mutex);
}

static bool rtcDateTimeMatches(const DS1302_DateTime &expected, const DS1302_DateTime &actual)
{
    if (expected.year != actual.year || expected.month != actual.month ||
        expected.dayMonth != actual.dayMonth || expected.hour != actual.hour ||
        expected.minute != actual.minute)
        return false;

    const int second_delta = (int)actual.second - (int)expected.second;
    return second_delta >= 0 && second_delta <= 2;
}

bool HAL::refreshTime()
{
    DS1302_DateTime snapshot = {};
    bool rtc_ok = false;
    if (!_rtc_fallback_to_system.load() && rtc_lock(pdMS_TO_TICKS(500)))
    {
        rtc_ok = DS1302_getDateTime(&rtc, &snapshot);
        rtc_unlock();
    }

    if (rtc_ok)
    {
        datetime = snapshot;
        return true;
    }

    const time_t utc_now = time(NULL);
    if (utc_now >= 1000000000)
    {
        const time_t local_now = utc_now + i18n::getNTPOffset();
        tm time_now = {};
        if (gmtime_r(&local_now, &time_now) != NULL)
        {
            snapshot.second = time_now.tm_sec;
            snapshot.minute = time_now.tm_min;
            snapshot.hour = time_now.tm_hour;
            snapshot.dayWeek = time_now.tm_wday == 0 ? 7 : time_now.tm_wday;
            snapshot.dayMonth = time_now.tm_mday;
            snapshot.month = time_now.tm_mon + 1;
            snapshot.year = time_now.tm_year + 1900;
            datetime = snapshot;
            return true;
        }
    }

    static uint32_t last_error_ms = 0;
    const uint32_t now_ms = millis();
    if (last_error_ms == 0 || now_ms - last_error_ms >= 5000)
    {
        ESP_LOGW("RTC", "RTC read failed and system time is unavailable; keeping the last snapshot");
        last_error_ms = now_ms;
    }
    return false;
}

void HAL::getTime()
{
    const uint32_t now_ms = millis();
    if (_last_rtc_request_ms != 0 && now_ms - _last_rtc_request_ms < RTC_READ_INTERVAL_MS)
        return;
    _last_rtc_request_ms = now_ms;

    if (_systemctl_task == NULL)
    {
        refreshTime();
        return;
    }

    sysctl_event_t event = {};
    event.type = EVENT_GET_TIME;
    if (xQueueSend(_queue, &event, 0) != pdTRUE)
        refreshTime();
}
uint8_t HAL::getDoW(uint16_t iYear, uint8_t iMonth, uint8_t iDay)
{
    int iWeek = 0;
    unsigned int y = 0, c = 0, m = 0, d = 0;

    if (iMonth == 1 || iMonth == 2)
    {
        c = (iYear - 1) / 100;
        y = (iYear - 1) % 100;
        m = iMonth + 12;
        d = iDay;
    }
    else
    {
        c = iYear / 100;
        y = iYear % 100;
        m = iMonth;
        d = iDay;
    }

    iWeek = y + y / 4 + c / 4 - 2 * c + 26 * (m + 1) / 10 + d - 1; // 蔡勒公式
    iWeek = iWeek >= 0 ? (iWeek % 7) : (iWeek % 7 + 7);            // iWeek为负时取模
    if (iWeek == 0)                                                // 星期日不作为一周的第一天
    {
        iWeek = 7;
    }

    return iWeek;
}
bool HAL::setTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
    if (year < 2000 || year > 2099 || month < 1 || month > 12 || day < 1 || day > 31 ||
        hour > 23 || minute > 59 || second > 59)
        return false;

    DS1302_DateTime dt = {};
    dt.second = second;
    dt.minute = minute;
    dt.hour = hour;
    dt.dayWeek = getDoW(year, month, day);
    dt.dayMonth = day;
    dt.month = month;
    dt.year = year;

    if (!rtc_lock(pdMS_TO_TICKS(1000)))
    {
        _rtc_fallback_to_system.store(true);
        datetime = dt;
        ESP_LOGE("RTC", "Timed out waiting to write the RTC");
        return false;
    }

    DS1302_writeProtect(&rtc, false);
    DS1302_halt(&rtc, false);
    DS1302_setDateTime(&rtc, &dt);
    DS1302_DateTime verified = {};
    const bool read_ok = DS1302_getDateTime(&rtc, &verified);
    const bool verified_ok = read_ok && rtcDateTimeMatches(dt, verified);
    rtc_unlock();

    _rtc_fallback_to_system.store(!verified_ok);
    datetime = verified_ok ? verified : dt;
    time_sync = true;
    if (!verified_ok)
        ESP_LOGW("RTC", "RTC write could not be verified; using the synchronized system clock");
    return verified_ok;
}

/*-------- NTP ----------*/
#define CONFIG_NTP_ADDR "ntp.aliyun.com"
#include "WiFiUdp.h"
#include "hal.h"
WiFiUDP Udp;
static const int NTP_PACKET_SIZE = 48;        // NTP time is in the first 48 bytes of message
static byte NTPpacketBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming & outgoing packets

// send an NTP request to the time server at the given address
static void sendNTPpacket(IPAddress &address)
{
    // set all bytes in the buffer to 0
    memset(NTPpacketBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    NTPpacketBuffer[0] = 0b11100011; // LI, Version, Mode
    NTPpacketBuffer[1] = 0;          // Stratum, or type of clock
    NTPpacketBuffer[2] = 6;          // Polling Interval
    NTPpacketBuffer[3] = 0xEC;       // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    NTPpacketBuffer[12] = 49;
    NTPpacketBuffer[13] = 0x4E;
    NTPpacketBuffer[14] = 49;
    NTPpacketBuffer[15] = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket(address, 123); // NTP requests are to port 123
    Udp.write(NTPpacketBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}
static time_t getNtpTime()
{
    IPAddress ntpServerIP;
    while (Udp.parsePacket() > 0)
        ;
    WiFi.hostByName(CONFIG_NTP_ADDR, ntpServerIP);
    sendNTPpacket(ntpServerIP);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500)
    {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE)
        {
            Udp.read(NTPpacketBuffer, NTP_PACKET_SIZE);
            unsigned long secsSince1900;
            secsSince1900 = (unsigned long)NTPpacketBuffer[40] << 24;
            secsSince1900 |= (unsigned long)NTPpacketBuffer[41] << 16;
            secsSince1900 |= (unsigned long)NTPpacketBuffer[42] << 8;
            secsSince1900 |= (unsigned long)NTPpacketBuffer[43];
            return secsSince1900 - 2208988800UL;
        }
    }
    return 0;
}
bool HAL::NTPSync()
{
    time_t utc_now = 0;
    Udp.begin(8888);
    for (uint8_t i = 0; i < 5; ++i)
    {
        delay(10);
        utc_now = getNtpTime();
        if (utc_now != 0 && utc_now != 0xffffffff)
        {
            timeval system_time = {};
            system_time.tv_sec = utc_now;
            if (settimeofday(&system_time, NULL) != 0)
            {
                ESP_LOGE("NTP", "设置系统时间失败: errno=%d", errno);
                utc_now = 0;
                continue;
            }

            time_t local_now = utc_now + i18n::getNTPOffset();
            tm time_now;
            gmtime_r(&local_now, &time_now);
            ESP_LOGI("NTP", "NTP同步成功: %d-%d-%d %d:%d:%d", time_now.tm_year + 1900, time_now.tm_mon + 1, time_now.tm_mday, time_now.tm_hour, time_now.tm_min, time_now.tm_sec);
            setTime(time_now.tm_year + 1900, time_now.tm_mon + 1, time_now.tm_mday, time_now.tm_hour, time_now.tm_min, time_now.tm_sec);
            break;
        }
        utc_now = 0;
    }
    Udp.stop();
    return utc_now != 0;
}

void HAL::setBrightness(int8_t brightness)
{
    static uint8_t brightness_lut[10] = {10, 15, 20, 30, 40, 60, 90, 130, 180, 255};
    if (brightness >= 0 && brightness <= 9)
    {
        _brightness = brightness;
        ledcWrite(PIN_DISPLAY_BL, brightness_lut[brightness]);
    }
    else
    {
        ESP_LOGE("HAL", "亮度设置错误: %d", brightness);
        ledcWrite(PIN_DISPLAY_BL, 130);
    }
}

void HAL::setVolume(int8_t volume)
{
    if (!audio_lock(portMAX_DELAY))
        return;

    static uint8_t volume_lut[10] = {0, 38, 46, 53, 59, 64, 68, 71, 73, 74};
    uint8_t codec_volume = AUDIO_VOICE_VOLUME;
    if (volume >= 0 && volume <= 9)
    {
        _volume = volume;
        codec_volume = volume_lut[volume];
    }
    else
    {
        ESP_LOGE("HAL", "音量设置错误: %d", volume);
    }

    if (!audio_ready || es_handle == NULL)
    {
        audio_unlock();
        return;
    }

    const esp_err_t err = es8311_voice_volume_set(es_handle, codec_volume, NULL);
    if (err != ESP_OK)
    {
        ESP_LOGE("HAL", "Volume write failed: %s (0x%x); muting amplifier",
                 esp_err_to_name(err), (unsigned)err);
        audio_ready = false;
        digitalWrite(AUDIO_AMP_CTRL, LOW);
        audio_unlock();
        return;
    }

    if (codec_volume == 0)
        digitalWrite(AUDIO_AMP_CTRL, LOW);
    audio_unlock();
}

void HAL::LOCKLV()
{
    xSemaphoreTake(_mutex, portMAX_DELAY);
}

void HAL::UNLOCKLV()
{
    xSemaphoreGive(_mutex);
}
#include <esp_sleep.h>
#include <driver/rtc_io.h>
void HAL::goSleep()
{
    xSemaphoreTake(appManagerLite._mutex, 2000);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_SERIAL1_RX, 0);
    rtc_gpio_pullup_en((gpio_num_t)PIN_SERIAL1_RX);
    rtc_gpio_hold_en((gpio_num_t)PIN_SERIAL1_RX);
    delay(2);
    esp_deep_sleep_start();
}

void HAL::send_sysctl(system_event_type_t type, uint8_t data)
{
    sysctl_event_t event;
    event.type = type;
    event.data = data;
    xQueueSend(_queue, &event, 20);
}
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

//////////////////////////////////网页服务器部分

#include <WebServer.h>
#include <ESPmDNS.h>
#include <errno.h>
#include <lwip/sockets.h>

#define FILESYSTEM LittleFS
DNSServer dnsServer;
WebServer server(80);
static bool webserver_routes_registered = false;
// holds the current upload
File fsUploadFile;
#include <list>

String getContentType(String filename)
{
    if (server.hasArg("download"))
    {
        return "application/octet-stream";
    }
    else if (filename.endsWith(".htm"))
    {
        return "text/html";
    }
    else if (filename.endsWith(".html"))
    {
        return "text/html";
    }
    else if (filename.endsWith(".css"))
    {
        return "text/css";
    }
    else if (filename.endsWith(".js"))
    {
        return "application/javascript";
    }
    else if (filename.endsWith(".png"))
    {
        return "image/png";
    }
    else if (filename.endsWith(".gif"))
    {
        return "image/gif";
    }
    else if (filename.endsWith(".jpg"))
    {
        return "image/jpeg";
    }
    else if (filename.endsWith(".ico"))
    {
        return "image/x-icon";
    }
    else if (filename.endsWith(".xml"))
    {
        return "text/xml";
    }
    else if (filename.endsWith(".pdf"))
    {
        return "application/x-pdf";
    }
    else if (filename.endsWith(".zip"))
    {
        return "application/x-zip";
    }
    else if (filename.endsWith(".gz"))
    {
        return "application/x-gzip";
    }
    return "text/plain";
}

bool exists(String path)
{
    bool yes = false;
    File file = FILESYSTEM.open(path, "r", false);
    if (!file.isDirectory())
    {
        yes = true;
    }
    file.close();
    return yes;
}

bool handleFileRead(String path)
{
    ESP_LOGI("SERVER", "handleFileRead: %s", path.c_str());
    if (path.endsWith(".wifi.csv"))
        return false;
    String contentType = getContentType(path);
    if (exists(path))
    {
        File file = FILESYSTEM.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}

static void sendGzippedAsset(const char *contentType, const uint8_t *content, size_t contentLength, bool immutable = true)
{
    server.sendHeader("Content-Encoding", "gzip", true);
    server.sendHeader("Cache-Control", immutable ? "public, max-age=31536000, immutable" : "no-cache");
    server.setContentLength(contentLength);
    server.send(200, contentType, "");

    NetworkClient &client = server.client();
    const int socketFd = client.fd();
    if (socketFd < 0)
        return;

    const size_t chunkSize = 512;
    const uint32_t idleTimeoutMs = 1200;
    uint32_t lastProgressMs = millis();
    size_t offset = 0;
    while (offset < contentLength && client.connected())
    {
        const size_t remaining = contentLength - offset;
        const size_t len = (remaining > chunkSize) ? chunkSize : remaining;
        int written = ::send(socketFd, content + offset, len, MSG_DONTWAIT);
        if (written > 0)
        {
            offset += (size_t)written;
            lastProgressMs = millis();
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        if (written < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
        {
            ESP_LOGW("SERVER", "asset send aborted errno=%d sent=%u/%u", errno, (unsigned int)offset, (unsigned int)contentLength);
            client.stop();
            return;
        }

        if (millis() - lastProgressMs > idleTimeoutMs)
        {
            ESP_LOGW("SERVER", "asset send timeout sent=%u/%u", (unsigned int)offset, (unsigned int)contentLength);
            client.stop();
            return;
        }

        fd_set writeSet;
        FD_ZERO(&writeSet);
        FD_SET(socketFd, &writeSet);
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 20000;
        select(socketFd + 1, NULL, &writeSet, NULL, &timeout);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void handleFileUpload()
{
    if (server.uri() != "/edit")
    {
        return;
    }
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        String filename = upload.filename;
        if (!filename.startsWith("/"))
        {
            filename = "/" + filename;
        }
        ESP_LOGI("SERVER", "handleFileUpload Name: %s", filename.c_str());
        fsUploadFile = FILESYSTEM.open(filename, "w");
        filename = String();
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (fsUploadFile)
        {
            fsUploadFile.write(upload.buf, upload.currentSize);
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (fsUploadFile)
        {
            if (strstr(fsUploadFile.name(), ".mpeg") != NULL)
                hal.gif_update = false;
            if (strstr(fsUploadFile.name(), ".jpg") != NULL)
                hal.jpg_update = false;
            fsUploadFile.close();
        }
        ESP_LOGI("SERVER", "handleFileUpload Size: %d", upload.totalSize);
    }
}

void handleFileDelete()
{
    if (server.args() == 0)
    {
        return server.send(500, "text/plain", "BAD ARGS");
    }
    String path = server.arg(0);
    ESP_LOGI("SERVER", "handleFileDelete: %s", path.c_str());
    if (path == "/")
    {
        return server.send(500, "text/plain", "BAD PATH");
    }
    if (!exists(path))
    {
        return server.send(404, "text/plain", "FileNotFound");
    }
    bool result = FILESYSTEM.remove(path);
    if (result) {
        server.send(200, "text/plain", "");
        if (strstr(path.c_str(), ".mpeg") != NULL)
            hal.gif_update = false;
        if (strstr(path.c_str(), ".jpg") != NULL)
            hal.jpg_update = false;
    } else {
        return server.send(500, "text/plain", "DELETE FAIL");  
    }
    path = String();
}

void handleFileCreate()
{
    if (server.args() == 0)
    {
        return server.send(500, "text/plain", "BAD ARGS");
    }
    String path = server.arg(0);
    ESP_LOGI("SERVER", "handleFileCreate: %s", path.c_str());
    if (path == "/")
    {
        return server.send(500, "text/plain", "BAD PATH");
    }
    if (exists(path))
    {
        return server.send(500, "text/plain", "FILE EXISTS");
    }
    File file = FILESYSTEM.open(path, "w");
    if (file)
    {
        file.close();
    }
    else
    {
        return server.send(500, "text/plain", "CREATE FAILED");
    }
    server.send(200, "text/plain", "");
    path = String();
}

void handleFileList()
{
    if (!server.hasArg("dir"))
    {
        server.send(500, "text/plain", "BAD ARGS");
        return;
    }

    String path = server.arg("dir");
    ESP_LOGI("SERVER", "handleFileList: %s", path.c_str());

    File root = FILESYSTEM.open(path);
    path = String();
    String output;
    bool first = true;
    output = "{\"size\":";
    output += String(FILESYSTEM.usedBytes());
    output += ",\"total\":";
    output += String(FILESYSTEM.totalBytes());
    output += ",\"data\":[";
    if (root.isDirectory())
    {
        File file = root.openNextFile();
        while (file)
        {
            if (file.name()[0] == '.')
            {
                file = root.openNextFile();
                continue;
            }
            if (first == false)
            {
                output += ',';
            }
            first = false;
            output += "{\"type\":\"";
            output += (file.isDirectory()) ? "dir" : "file";
            output += "\",\"name\":\"";
            output += String(file.name());
            output += "\",\"size\":\"";
            output += String(file.size());
            output += "\"}";
            file = root.openNextFile();
        }
    }
    output += "]}";
    server.send(200, "text/json", output);
}

void handleRMRF()
{
    server.send(500, "text/plain", "此功能已被移除");
}
void handleRename()
{
    server.send(500, "text/plain", "此功能已被移除");
}
void handleMkdir()
{
    server.send(500, "text/plain", "此功能已被移除");
}
void handleTime()
{
    if (server.hasArg("plain"))
    {
        String time = server.arg("plain");
        const time_t utc_now = (time_t)atoll(time.c_str());
        if (utc_now > 0)
        {
            timeval tv = {};
            tv.tv_sec = utc_now;
            if (settimeofday(&tv, NULL) != 0)
            {
                ESP_LOGE("TIME", "Browser time sync failed: errno=%d", errno);
                server.send(500, "text/plain", "ERR 500");
                return;
            }

            const time_t local_now = utc_now + i18n::getNTPOffset();
            tm newtime = {};
            gmtime_r(&local_now, &newtime);
            if (!hal.setTime(newtime.tm_year + 1900, newtime.tm_mon + 1, newtime.tm_mday,
                             newtime.tm_hour, newtime.tm_min, newtime.tm_sec))
                ESP_LOGW("TIME", "Browser time was applied to the system clock but RTC verification failed");
            server.send(200, "text/plain", "OK");
            return;
        }
    }
    server.send(500, "text/plain", "ERR 500");
}
//////////////////////////////////主页APP JSON处理
#include <cJSON.h>

static char jsonbuffer[16384];
const char default_weather_key[] = "SoC098cCa8Ih-GWTb";
const char default_weather_city[] = "北京";
const char default_weather_provider[] = "aliyun_72158";
const char default_weather_endpoint[] = "https://getweather.market.alicloudapi.com/lundear/weather1d";
const char default_weather_lat[] = "39.9042";
const char default_weather_lon[] = "116.4074";
const char default_app_setting[] = "{\"weather\":\"\",\"city\":\"北京\",\"weather_provider\":\"aliyun_72158\",\"weather_endpoint\":\"https://getweather.market.alicloudapi.com/lundear/weather1d\",\"weather_lat\":\"39.9042\",\"weather_lon\":\"116.4074\"}";

struct app_setting app_settings_save;

struct app_setting_v2
{
    char weather_secret[64];
    char weather_city[64];
    char weather_provider[16];
    char weather_endpoint[96];
    char weather_lat[24];
    char weather_lon[24];
};

struct app_setting_v3
{
    char weather_secret[64];
    char weather_city[64];
    char weather_provider[32];
    char weather_endpoint[128];
    char weather_lat[24];
    char weather_lon[24];
    char weather_key_seniverse[64];
    char weather_key_qweather[64];
    char weather_key_aliyun_72158[64];
    char weather_key_aliyun_10812[64];
    char weather_key_aliyun_50139[64];
    char weather_key_aliyun_71988[64];
};

struct app_setting_v5
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
};
struct app_setting_v4
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
};

struct extended_app_setting
{
    char remote_ip[64];
    uint16_t remote_port;
    char weather_secret[64];
    char weather_city[64];
    char weather_provider[16];
    char weather_endpoint[96];
    char weather_lat[24];
    char weather_lon[24];
    char userdata[256];
};

struct legacy_app_setting
{
    char remote_ip[64];
    uint16_t remote_port;
    char weather_secret[64];
    char weather_city[64];
    char userdata[256];
};

union app_settings_scratch_storage
{
    app_setting current;
    app_setting_v5 v5;
    app_setting_v4 v4;
    app_setting_v3 v3;
    app_setting_v2 v2;
    extended_app_setting extended;
    legacy_app_setting legacy;
};

static app_settings_scratch_storage app_settings_scratch;
static_assert(sizeof(app_settings_scratch_storage) == sizeof(app_setting),
              "Settings scratch storage must remain bounded by the current format");

template <size_t N>
static void terminateSettingString(char (&value)[N])
{
    value[N - 1] = '\0';
}

template <typename T>
static void sanitizeBaseWeatherFields(T &settings)
{
    terminateSettingString(settings.weather_secret);
    terminateSettingString(settings.weather_city);
    terminateSettingString(settings.weather_provider);
    terminateSettingString(settings.weather_endpoint);
    terminateSettingString(settings.weather_lat);
    terminateSettingString(settings.weather_lon);
}

template <typename T>
static void sanitizeWeatherEndpointFields(T &settings)
{
    terminateSettingString(settings.weather_endpoint_seniverse);
    terminateSettingString(settings.weather_endpoint_qweather);
    terminateSettingString(settings.weather_endpoint_aliyun_72158);
    terminateSettingString(settings.weather_endpoint_aliyun_10812);
    terminateSettingString(settings.weather_endpoint_aliyun_50139);
    terminateSettingString(settings.weather_endpoint_aliyun_71988);
}

template <typename T>
static void sanitizeWeatherKeyFields(T &settings)
{
    terminateSettingString(settings.weather_key_seniverse);
    terminateSettingString(settings.weather_key_qweather);
    terminateSettingString(settings.weather_key_aliyun_72158);
    terminateSettingString(settings.weather_key_aliyun_10812);
    terminateSettingString(settings.weather_key_aliyun_50139);
    terminateSettingString(settings.weather_key_aliyun_71988);
}

template <typename T>
static void sanitizeAliyunCredentialFields(T &settings)
{
    terminateSettingString(settings.weather_appkey_aliyun_72158);
    terminateSettingString(settings.weather_appkey_aliyun_10812);
    terminateSettingString(settings.weather_appkey_aliyun_50139);
    terminateSettingString(settings.weather_appkey_aliyun_71988);
    terminateSettingString(settings.weather_appsecret_aliyun_72158);
    terminateSettingString(settings.weather_appsecret_aliyun_10812);
    terminateSettingString(settings.weather_appsecret_aliyun_50139);
    terminateSettingString(settings.weather_appsecret_aliyun_71988);
}

static void sanitizeAppSettingStrings(app_setting &settings)
{
    sanitizeBaseWeatherFields(settings);
    sanitizeWeatherEndpointFields(settings);
    sanitizeWeatherKeyFields(settings);
    sanitizeAliyunCredentialFields(settings);
    terminateSettingString(settings.weather_publickey_seniverse);
}

static constexpr uint32_t APP_SETTINGS_MAGIC = 0x534C4346u;
static constexpr uint16_t APP_SETTINGS_FORMAT_VERSION = 1;

struct app_settings_file_header
{
    uint32_t magic;
    uint16_t version;
    uint16_t payload_size;
    uint32_t checksum;
};

static_assert(sizeof(app_settings_file_header) == 12, "Unexpected app settings header layout");

static uint32_t settingsChecksum(const uint8_t *data, size_t size)
{
    uint32_t checksum = 2166136261u;
    for (size_t i = 0; i < size; i++)
    {
        checksum ^= data[i];
        checksum *= 16777619u;
    }
    return checksum;
}

static bool readCurrentAppSettings(File &file, app_setting &loaded)
{
    if (file.size() != sizeof(app_settings_file_header) + sizeof(loaded))
        return false;

    app_settings_file_header header = {};
    if (file.read((uint8_t *)&header, sizeof(header)) != sizeof(header))
        return false;
    if (header.magic != APP_SETTINGS_MAGIC ||
        header.version != APP_SETTINGS_FORMAT_VERSION ||
        header.payload_size != sizeof(loaded))
        return false;

    memset(&loaded, 0, sizeof(loaded));
    if (file.read((uint8_t *)&loaded, sizeof(loaded)) != sizeof(loaded))
        return false;
    if (settingsChecksum((const uint8_t *)&loaded, sizeof(loaded)) != header.checksum)
        return false;

    sanitizeAppSettingStrings(loaded);
    return true;
}

static bool writeCurrentAppSettings(const app_setting &settings)
{
    app_settings_file_header header = {
        APP_SETTINGS_MAGIC,
        APP_SETTINGS_FORMAT_VERSION,
        (uint16_t)sizeof(settings),
        settingsChecksum((const uint8_t *)&settings, sizeof(settings)),
    };

    static const char *const tempPath = "/.cfg.bin.tmp";
    static const char *const targetPath = "/.cfg.bin";
    LittleFS.remove(tempPath);
    File file = LittleFS.open(tempPath, "w");
    if (!file)
        return false;

    const size_t headerWritten = file.write((const uint8_t *)&header, sizeof(header));
    const size_t payloadWritten = file.write((const uint8_t *)&settings, sizeof(settings));
    file.flush();
    file.close();
    if (headerWritten != sizeof(header) || payloadWritten != sizeof(settings))
    {
        LittleFS.remove(tempPath);
        return false;
    }

    if (!LittleFS.rename(tempPath, targetPath))
    {
        LittleFS.remove(targetPath);
        if (!LittleFS.rename(tempPath, targetPath))
        {
            LittleFS.remove(tempPath);
            return false;
        }
    }
    return true;
}

static void sanitizeAppSettingStrings(app_setting_v5 &settings)
{
    sanitizeBaseWeatherFields(settings);
    sanitizeWeatherEndpointFields(settings);
    sanitizeWeatherKeyFields(settings);
    sanitizeAliyunCredentialFields(settings);
}

static void sanitizeAppSettingStrings(app_setting_v4 &settings)
{
    sanitizeBaseWeatherFields(settings);
    sanitizeWeatherEndpointFields(settings);
    sanitizeWeatherKeyFields(settings);
}

static void sanitizeAppSettingStrings(app_setting_v3 &settings)
{
    sanitizeBaseWeatherFields(settings);
    sanitizeWeatherKeyFields(settings);
}

static void sanitizeAppSettingStrings(app_setting_v2 &settings)
{
    sanitizeBaseWeatherFields(settings);
}

static void sanitizeAppSettingStrings(extended_app_setting &settings)
{
    sanitizeBaseWeatherFields(settings);
    terminateSettingString(settings.remote_ip);
    terminateSettingString(settings.userdata);
}

static void sanitizeAppSettingStrings(legacy_app_setting &settings)
{
    terminateSettingString(settings.remote_ip);
    terminateSettingString(settings.weather_secret);
    terminateSettingString(settings.weather_city);
    terminateSettingString(settings.userdata);
}

static bool writeJsonToBuffer(cJSON *json, char *result, size_t result_size)
{
    if (json == NULL || result == NULL || result_size == 0)
        return false;

    char *printed = cJSON_PrintUnformatted(json);
    if (printed == NULL)
    {
        result[0] = '\0';
        return false;
    }

    strncpy(result, printed, result_size - 1);
    result[result_size - 1] = '\0';
    cJSON_free(printed);
    return true;
}

static void addHeapDiagnostics(cJSON *json)
{
    cJSON_AddNumberToObject(json, "heap_free", ESP.getFreeHeap());
    cJSON_AddNumberToObject(json, "heap_internal_free", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    cJSON_AddNumberToObject(json, "heap_largest_internal", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    cJSON_AddNumberToObject(json, "heap_spiram_free", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    cJSON_AddNumberToObject(json, "heap_largest_spiram", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
}

static void copySettingString(char *dst, size_t dst_size, const char *value)
{
    if (dst == NULL || dst_size == 0 || value == NULL)
        return;
    strncpy(dst, value, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

static const char *jsonStringValue(cJSON *root, const char *key)
{
    cJSON *item = cJSON_GetObjectItem(root, key);
    return (item != NULL && cJSON_IsString(item) && item->valuestring != NULL) ? item->valuestring : NULL;
}

void parseAppSettings(const char *input);

static bool isKnownWeatherProvider(const char *provider)
{
    return provider != NULL &&
           (strcmp(provider, "seniverse") == 0 ||
            strcmp(provider, "qweather") == 0 ||
            strcmp(provider, "aliyun_72158") == 0 ||
            strcmp(provider, "aliyun_10812") == 0 ||
            strcmp(provider, "aliyun_50139") == 0 ||
            strcmp(provider, "aliyun_71988") == 0);
}

static bool isAliyunWeatherProviderName(const char *provider)
{
    return provider != NULL &&
           (strcmp(provider, "aliyun_72158") == 0 ||
            strcmp(provider, "aliyun_10812") == 0 ||
            strcmp(provider, "aliyun_50139") == 0 ||
            strcmp(provider, "aliyun_71988") == 0);
}

static const char *defaultWeatherEndpointForProvider(const char *provider)
{
    if (provider != NULL && strcmp(provider, "seniverse") == 0)
        return "http://api.seniverse.com";
    if (provider != NULL && strcmp(provider, "qweather") == 0)
        return "https://devapi.qweather.com";
    if (provider != NULL && strcmp(provider, "aliyun_10812") == 0)
        return "https://ali-weather.showapi.com/spot-to-weather";
    if (provider != NULL && strcmp(provider, "aliyun_50139") == 0)
        return "https://weather01.market.alicloudapi.com/weather";
    if (provider != NULL && strcmp(provider, "aliyun_71988") == 0)
        return "https://kzweather.market.alicloudapi.com/weather";
    return default_weather_endpoint;
}

static char *weatherKeySlotForProvider(const char *provider)
{
    if (provider == NULL)
        return NULL;
    if (strcmp(provider, "seniverse") == 0)
        return app_settings_save.weather_key_seniverse;
    if (strcmp(provider, "qweather") == 0)
        return app_settings_save.weather_key_qweather;
    if (strcmp(provider, "aliyun_72158") == 0)
        return app_settings_save.weather_key_aliyun_72158;
    if (strcmp(provider, "aliyun_10812") == 0)
        return app_settings_save.weather_key_aliyun_10812;
    if (strcmp(provider, "aliyun_50139") == 0)
        return app_settings_save.weather_key_aliyun_50139;
    if (strcmp(provider, "aliyun_71988") == 0)
        return app_settings_save.weather_key_aliyun_71988;
    return NULL;
}

static char *weatherEndpointSlotForProvider(const char *provider)
{
    if (provider == NULL)
        return NULL;
    if (strcmp(provider, "seniverse") == 0)
        return app_settings_save.weather_endpoint_seniverse;
    if (strcmp(provider, "qweather") == 0)
        return app_settings_save.weather_endpoint_qweather;
    if (strcmp(provider, "aliyun_72158") == 0)
        return app_settings_save.weather_endpoint_aliyun_72158;
    if (strcmp(provider, "aliyun_10812") == 0)
        return app_settings_save.weather_endpoint_aliyun_10812;
    if (strcmp(provider, "aliyun_50139") == 0)
        return app_settings_save.weather_endpoint_aliyun_50139;
    if (strcmp(provider, "aliyun_71988") == 0)
        return app_settings_save.weather_endpoint_aliyun_71988;
    return NULL;
}

static char *weatherAppKeySlotForProvider(const char *provider)
{
    if (provider == NULL)
        return NULL;
    if (strcmp(provider, "aliyun_72158") == 0)
        return app_settings_save.weather_appkey_aliyun_72158;
    if (strcmp(provider, "aliyun_10812") == 0)
        return app_settings_save.weather_appkey_aliyun_10812;
    if (strcmp(provider, "aliyun_50139") == 0)
        return app_settings_save.weather_appkey_aliyun_50139;
    if (strcmp(provider, "aliyun_71988") == 0)
        return app_settings_save.weather_appkey_aliyun_71988;
    return NULL;
}

static char *weatherAppSecretSlotForProvider(const char *provider)
{
    if (provider == NULL)
        return NULL;
    if (strcmp(provider, "aliyun_72158") == 0)
        return app_settings_save.weather_appsecret_aliyun_72158;
    if (strcmp(provider, "aliyun_10812") == 0)
        return app_settings_save.weather_appsecret_aliyun_10812;
    if (strcmp(provider, "aliyun_50139") == 0)
        return app_settings_save.weather_appsecret_aliyun_50139;
    if (strcmp(provider, "aliyun_71988") == 0)
        return app_settings_save.weather_appsecret_aliyun_71988;
    return NULL;
}

static const char *constWeatherKeySlotForProvider(const char *provider)
{
    char *slot = weatherKeySlotForProvider(provider);
    return slot == NULL ? "" : slot;
}

static const char *constWeatherEndpointSlotForProvider(const char *provider)
{
    char *slot = weatherEndpointSlotForProvider(provider);
    return slot == NULL ? "" : slot;
}

static const char *constWeatherAppKeySlotForProvider(const char *provider)
{
    char *slot = weatherAppKeySlotForProvider(provider);
    return slot == NULL ? "" : slot;
}

static const char *constWeatherAppSecretSlotForProvider(const char *provider)
{
    char *slot = weatherAppSecretSlotForProvider(provider);
    return slot == NULL ? "" : slot;
}

static void syncActiveWeatherKeyFromProvider()
{
    const char *saved = constWeatherKeySlotForProvider(app_settings_save.weather_provider);
    if (saved != NULL && strlen(saved) > 0)
        copySettingString(app_settings_save.weather_secret, sizeof(app_settings_save.weather_secret), saved);
    else
        app_settings_save.weather_secret[0] = '\0';
}

static void syncActiveWeatherEndpointFromProvider()
{
    const char *saved = constWeatherEndpointSlotForProvider(app_settings_save.weather_provider);
    if (saved != NULL && strlen(saved) > 0)
        copySettingString(app_settings_save.weather_endpoint, sizeof(app_settings_save.weather_endpoint), saved);
    else
        copySettingString(app_settings_save.weather_endpoint, sizeof(app_settings_save.weather_endpoint), defaultWeatherEndpointForProvider(app_settings_save.weather_provider));
}

static void saveActiveWeatherKeyForProvider(const char *provider, const char *key)
{
    char *slot = weatherKeySlotForProvider(provider);
    if (slot == NULL || key == NULL)
        return;
    copySettingString(slot, 64, key);
    copySettingString(app_settings_save.weather_secret, sizeof(app_settings_save.weather_secret), key);
}

static void saveAliyunAppKeyForProvider(const char *provider, const char *appKey)
{
    char *slot = weatherAppKeySlotForProvider(provider);
    if (slot == NULL || appKey == NULL)
        return;
    copySettingString(slot, 64, appKey);
}

static void saveAliyunAppSecretForProvider(const char *provider, const char *appSecret)
{
    char *slot = weatherAppSecretSlotForProvider(provider);
    if (slot == NULL || appSecret == NULL)
        return;
    copySettingString(slot, 64, appSecret);
}

static void saveActiveWeatherEndpointForProvider(const char *provider, const char *endpoint)
{
    char *slot = weatherEndpointSlotForProvider(provider);
    if (slot == NULL || endpoint == NULL || strlen(endpoint) == 0)
        return;
    copySettingString(slot, 128, endpoint);
    if (strcmp(provider, app_settings_save.weather_provider) == 0)
        copySettingString(app_settings_save.weather_endpoint, sizeof(app_settings_save.weather_endpoint), endpoint);
}

static bool isProviderKeyConfigured(const char *provider)
{
    const char *saved = constWeatherKeySlotForProvider(provider);
    if (saved != NULL && strlen(saved) > 0)
        return true;
    if (!isAliyunWeatherProviderName(provider))
        return false;
    return strlen(constWeatherAppKeySlotForProvider(provider)) > 0 &&
           strlen(constWeatherAppSecretSlotForProvider(provider)) > 0;
}

static void fillWeatherEndpointDefaults()
{
    const char *providers[] = {
        "seniverse",
        "qweather",
        "aliyun_72158",
        "aliyun_10812",
        "aliyun_50139",
        "aliyun_71988",
    };
    for (size_t i = 0; i < sizeof(providers) / sizeof(providers[0]); i++)
    {
        char *slot = weatherEndpointSlotForProvider(providers[i]);
        if (slot != NULL && strlen(slot) == 0)
            copySettingString(slot, 128, defaultWeatherEndpointForProvider(providers[i]));
    }
}

static void parseWeatherEndpointField(cJSON *root, const char *provider, const char *field)
{
    const char *endpoint = jsonStringValue(root, field);
    if (endpoint != NULL && strlen(endpoint) > 0)
        saveActiveWeatherEndpointForProvider(provider, endpoint);
}

static void parseWeatherEndpointMap(cJSON *root)
{
    cJSON *map = cJSON_GetObjectItem(root, "weather_provider_endpoints");
    if (map == NULL || !cJSON_IsObject(map))
        return;

    const char *providers[] = {
        "seniverse",
        "qweather",
        "aliyun_72158",
        "aliyun_10812",
        "aliyun_50139",
        "aliyun_71988",
    };
    for (size_t i = 0; i < sizeof(providers) / sizeof(providers[0]); i++)
    {
        const char *endpoint = jsonStringValue(map, providers[i]);
        if (endpoint != NULL && strlen(endpoint) > 0)
            saveActiveWeatherEndpointForProvider(providers[i], endpoint);
    }
}

static void parseWeatherKeyField(cJSON *root, const char *provider, const char *field)
{
    const char *key = jsonStringValue(root, field);
    if (key != NULL)
        saveActiveWeatherKeyForProvider(provider, key);
}

static void parseWeatherKeyMap(cJSON *root)
{
    cJSON *map = cJSON_GetObjectItem(root, "weather_provider_key_values");
    if (map == NULL || !cJSON_IsObject(map))
        return;

    const char *providers[] = {
        "seniverse",
        "qweather",
        "aliyun_72158",
        "aliyun_10812",
        "aliyun_50139",
        "aliyun_71988",
    };
    for (size_t i = 0; i < sizeof(providers) / sizeof(providers[0]); i++)
    {
        const char *key = jsonStringValue(map, providers[i]);
        if (key != NULL)
            saveActiveWeatherKeyForProvider(providers[i], key);
    }
}

static void parseAliyunCredentialMap(cJSON *root)
{
    cJSON *map = cJSON_GetObjectItem(root, "weather_provider_credentials");
    if (map == NULL || !cJSON_IsObject(map))
        return;

    const char *providers[] = {
        "aliyun_72158",
        "aliyun_10812",
        "aliyun_50139",
        "aliyun_71988",
    };
    for (size_t i = 0; i < sizeof(providers) / sizeof(providers[0]); i++)
    {
        cJSON *item = cJSON_GetObjectItem(map, providers[i]);
        if (item == NULL || !cJSON_IsObject(item))
            continue;
        const char *appCode = jsonStringValue(item, "appcode");
        if (appCode == NULL)
            appCode = jsonStringValue(item, "appCode");
        if (appCode != NULL)
            saveActiveWeatherKeyForProvider(providers[i], appCode);
        const char *appKey = jsonStringValue(item, "appkey");
        if (appKey == NULL)
            appKey = jsonStringValue(item, "appKey");
        if (appKey != NULL)
            saveAliyunAppKeyForProvider(providers[i], appKey);
        const char *appSecret = jsonStringValue(item, "appsecret");
        if (appSecret == NULL)
            appSecret = jsonStringValue(item, "appSecret");
        if (appSecret != NULL)
            saveAliyunAppSecretForProvider(providers[i], appSecret);
    }
}

static void parseAliyunCredentialFields(cJSON *root, const char *provider)
{
    if (!isAliyunWeatherProviderName(provider))
        return;

    char field[64];
    snprintf(field, sizeof(field), "weather_appcode_%s", provider);
    const char *appCode = jsonStringValue(root, field);
    if (appCode != NULL)
        saveActiveWeatherKeyForProvider(provider, appCode);

    snprintf(field, sizeof(field), "weather_appkey_%s", provider);
    const char *appKey = jsonStringValue(root, field);
    if (appKey != NULL)
        saveAliyunAppKeyForProvider(provider, appKey);

    snprintf(field, sizeof(field), "weather_appsecret_%s", provider);
    const char *appSecret = jsonStringValue(root, field);
    if (appSecret != NULL)
        saveAliyunAppSecretForProvider(provider, appSecret);
}

static void fillWeatherDefaultsIfMissing()
{
    if (!isKnownWeatherProvider(app_settings_save.weather_provider))
        copySettingString(app_settings_save.weather_provider, sizeof(app_settings_save.weather_provider), default_weather_provider);

    fillWeatherEndpointDefaults();
    syncActiveWeatherEndpointFromProvider();

    if (strlen(app_settings_save.weather_city) == 0)
        copySettingString(app_settings_save.weather_city, sizeof(app_settings_save.weather_city), default_weather_city);
    if (strcmp(app_settings_save.weather_city, default_weather_city) == 0 && strlen(app_settings_save.weather_lat) == 0)
        copySettingString(app_settings_save.weather_lat, sizeof(app_settings_save.weather_lat), default_weather_lat);
    if (strcmp(app_settings_save.weather_city, default_weather_city) == 0 && strlen(app_settings_save.weather_lon) == 0)
        copySettingString(app_settings_save.weather_lon, sizeof(app_settings_save.weather_lon), default_weather_lon);
    syncActiveWeatherKeyFromProvider();
}

static void migrateLegacyAppSettings(const legacy_app_setting &legacy)
{
    parseAppSettings(default_app_setting);
    copySettingString(app_settings_save.weather_city, sizeof(app_settings_save.weather_city), legacy.weather_city);

    if (strlen(legacy.weather_secret) > 0 && strcmp(legacy.weather_secret, default_weather_key) != 0)
    {
        copySettingString(app_settings_save.weather_secret, sizeof(app_settings_save.weather_secret), legacy.weather_secret);
        copySettingString(app_settings_save.weather_provider, sizeof(app_settings_save.weather_provider), "seniverse");
        copySettingString(app_settings_save.weather_endpoint, sizeof(app_settings_save.weather_endpoint), "http://api.seniverse.com");
        saveActiveWeatherEndpointForProvider("seniverse", "http://api.seniverse.com");
        saveActiveWeatherKeyForProvider("seniverse", legacy.weather_secret);
    }
    else
    {
        app_settings_save.weather_secret[0] = '\0';
        copySettingString(app_settings_save.weather_provider, sizeof(app_settings_save.weather_provider), default_weather_provider);
        copySettingString(app_settings_save.weather_endpoint, sizeof(app_settings_save.weather_endpoint), default_weather_endpoint);
        saveActiveWeatherEndpointForProvider(default_weather_provider, default_weather_endpoint);
        app_settings_save.weather_lat[0] = '\0';
        app_settings_save.weather_lon[0] = '\0';
        if (strlen(app_settings_save.weather_city) == 0 || strcmp(app_settings_save.weather_city, default_weather_city) == 0)
        {
            copySettingString(app_settings_save.weather_lat, sizeof(app_settings_save.weather_lat), default_weather_lat);
            copySettingString(app_settings_save.weather_lon, sizeof(app_settings_save.weather_lon), default_weather_lon);
        }
    }
}

static void migrateExtendedAppSettings(const extended_app_setting &legacy)
{
    parseAppSettings(default_app_setting);
    copySettingString(app_settings_save.weather_secret, sizeof(app_settings_save.weather_secret), legacy.weather_secret);
    copySettingString(app_settings_save.weather_city, sizeof(app_settings_save.weather_city), legacy.weather_city);
    copySettingString(app_settings_save.weather_provider, sizeof(app_settings_save.weather_provider), legacy.weather_provider);
    copySettingString(app_settings_save.weather_endpoint, sizeof(app_settings_save.weather_endpoint), legacy.weather_endpoint);
    saveActiveWeatherEndpointForProvider(app_settings_save.weather_provider, legacy.weather_endpoint);
    copySettingString(app_settings_save.weather_lat, sizeof(app_settings_save.weather_lat), legacy.weather_lat);
    copySettingString(app_settings_save.weather_lon, sizeof(app_settings_save.weather_lon), legacy.weather_lon);
    if (strlen(legacy.weather_secret) > 0 && strcmp(legacy.weather_secret, default_weather_key) != 0)
        saveActiveWeatherKeyForProvider(app_settings_save.weather_provider, legacy.weather_secret);
    fillWeatherDefaultsIfMissing();
}

static void migrateAppSettingsV2(const app_setting_v2 &legacy)
{
    parseAppSettings(default_app_setting);
    copySettingString(app_settings_save.weather_city, sizeof(app_settings_save.weather_city), legacy.weather_city);
    copySettingString(app_settings_save.weather_provider, sizeof(app_settings_save.weather_provider), legacy.weather_provider);
    copySettingString(app_settings_save.weather_endpoint, sizeof(app_settings_save.weather_endpoint), legacy.weather_endpoint);
    saveActiveWeatherEndpointForProvider(app_settings_save.weather_provider, legacy.weather_endpoint);
    copySettingString(app_settings_save.weather_lat, sizeof(app_settings_save.weather_lat), legacy.weather_lat);
    copySettingString(app_settings_save.weather_lon, sizeof(app_settings_save.weather_lon), legacy.weather_lon);
    if (strlen(legacy.weather_secret) > 0 && strcmp(legacy.weather_secret, default_weather_key) != 0)
        saveActiveWeatherKeyForProvider(app_settings_save.weather_provider, legacy.weather_secret);
    fillWeatherDefaultsIfMissing();
}

static void migrateAppSettingsV3(const app_setting_v3 &legacy)
{
    parseAppSettings(default_app_setting);
    copySettingString(app_settings_save.weather_secret, sizeof(app_settings_save.weather_secret), legacy.weather_secret);
    copySettingString(app_settings_save.weather_city, sizeof(app_settings_save.weather_city), legacy.weather_city);
    copySettingString(app_settings_save.weather_provider, sizeof(app_settings_save.weather_provider), legacy.weather_provider);
    copySettingString(app_settings_save.weather_endpoint, sizeof(app_settings_save.weather_endpoint), legacy.weather_endpoint);
    saveActiveWeatherEndpointForProvider(app_settings_save.weather_provider, legacy.weather_endpoint);
    copySettingString(app_settings_save.weather_lat, sizeof(app_settings_save.weather_lat), legacy.weather_lat);
    copySettingString(app_settings_save.weather_lon, sizeof(app_settings_save.weather_lon), legacy.weather_lon);
    copySettingString(app_settings_save.weather_key_seniverse, sizeof(app_settings_save.weather_key_seniverse), legacy.weather_key_seniverse);
    copySettingString(app_settings_save.weather_key_qweather, sizeof(app_settings_save.weather_key_qweather), legacy.weather_key_qweather);
    copySettingString(app_settings_save.weather_key_aliyun_72158, sizeof(app_settings_save.weather_key_aliyun_72158), legacy.weather_key_aliyun_72158);
    copySettingString(app_settings_save.weather_key_aliyun_10812, sizeof(app_settings_save.weather_key_aliyun_10812), legacy.weather_key_aliyun_10812);
    copySettingString(app_settings_save.weather_key_aliyun_50139, sizeof(app_settings_save.weather_key_aliyun_50139), legacy.weather_key_aliyun_50139);
    copySettingString(app_settings_save.weather_key_aliyun_71988, sizeof(app_settings_save.weather_key_aliyun_71988), legacy.weather_key_aliyun_71988);
    if (strlen(legacy.weather_secret) > 0 && strcmp(legacy.weather_secret, default_weather_key) != 0)
        saveActiveWeatherKeyForProvider(app_settings_save.weather_provider, legacy.weather_secret);
    fillWeatherDefaultsIfMissing();
}

static void migrateAppSettingsV5(const app_setting_v5 &legacy)
{
    memset(&app_settings_save, 0, sizeof(app_settings_save));
    copySettingString(app_settings_save.weather_secret, sizeof(app_settings_save.weather_secret), legacy.weather_secret);
    copySettingString(app_settings_save.weather_city, sizeof(app_settings_save.weather_city), legacy.weather_city);
    copySettingString(app_settings_save.weather_provider, sizeof(app_settings_save.weather_provider), legacy.weather_provider);
    copySettingString(app_settings_save.weather_endpoint, sizeof(app_settings_save.weather_endpoint), legacy.weather_endpoint);
    copySettingString(app_settings_save.weather_endpoint_seniverse, sizeof(app_settings_save.weather_endpoint_seniverse), legacy.weather_endpoint_seniverse);
    copySettingString(app_settings_save.weather_endpoint_qweather, sizeof(app_settings_save.weather_endpoint_qweather), legacy.weather_endpoint_qweather);
    copySettingString(app_settings_save.weather_endpoint_aliyun_72158, sizeof(app_settings_save.weather_endpoint_aliyun_72158), legacy.weather_endpoint_aliyun_72158);
    copySettingString(app_settings_save.weather_endpoint_aliyun_10812, sizeof(app_settings_save.weather_endpoint_aliyun_10812), legacy.weather_endpoint_aliyun_10812);
    copySettingString(app_settings_save.weather_endpoint_aliyun_50139, sizeof(app_settings_save.weather_endpoint_aliyun_50139), legacy.weather_endpoint_aliyun_50139);
    copySettingString(app_settings_save.weather_endpoint_aliyun_71988, sizeof(app_settings_save.weather_endpoint_aliyun_71988), legacy.weather_endpoint_aliyun_71988);
    copySettingString(app_settings_save.weather_lat, sizeof(app_settings_save.weather_lat), legacy.weather_lat);
    copySettingString(app_settings_save.weather_lon, sizeof(app_settings_save.weather_lon), legacy.weather_lon);
    copySettingString(app_settings_save.weather_key_seniverse, sizeof(app_settings_save.weather_key_seniverse), legacy.weather_key_seniverse);
    copySettingString(app_settings_save.weather_key_qweather, sizeof(app_settings_save.weather_key_qweather), legacy.weather_key_qweather);
    copySettingString(app_settings_save.weather_key_aliyun_72158, sizeof(app_settings_save.weather_key_aliyun_72158), legacy.weather_key_aliyun_72158);
    copySettingString(app_settings_save.weather_key_aliyun_10812, sizeof(app_settings_save.weather_key_aliyun_10812), legacy.weather_key_aliyun_10812);
    copySettingString(app_settings_save.weather_key_aliyun_50139, sizeof(app_settings_save.weather_key_aliyun_50139), legacy.weather_key_aliyun_50139);
    copySettingString(app_settings_save.weather_key_aliyun_71988, sizeof(app_settings_save.weather_key_aliyun_71988), legacy.weather_key_aliyun_71988);
    copySettingString(app_settings_save.weather_appkey_aliyun_72158, sizeof(app_settings_save.weather_appkey_aliyun_72158), legacy.weather_appkey_aliyun_72158);
    copySettingString(app_settings_save.weather_appkey_aliyun_10812, sizeof(app_settings_save.weather_appkey_aliyun_10812), legacy.weather_appkey_aliyun_10812);
    copySettingString(app_settings_save.weather_appkey_aliyun_50139, sizeof(app_settings_save.weather_appkey_aliyun_50139), legacy.weather_appkey_aliyun_50139);
    copySettingString(app_settings_save.weather_appkey_aliyun_71988, sizeof(app_settings_save.weather_appkey_aliyun_71988), legacy.weather_appkey_aliyun_71988);
    copySettingString(app_settings_save.weather_appsecret_aliyun_72158, sizeof(app_settings_save.weather_appsecret_aliyun_72158), legacy.weather_appsecret_aliyun_72158);
    copySettingString(app_settings_save.weather_appsecret_aliyun_10812, sizeof(app_settings_save.weather_appsecret_aliyun_10812), legacy.weather_appsecret_aliyun_10812);
    copySettingString(app_settings_save.weather_appsecret_aliyun_50139, sizeof(app_settings_save.weather_appsecret_aliyun_50139), legacy.weather_appsecret_aliyun_50139);
    copySettingString(app_settings_save.weather_appsecret_aliyun_71988, sizeof(app_settings_save.weather_appsecret_aliyun_71988), legacy.weather_appsecret_aliyun_71988);
    fillWeatherDefaultsIfMissing();
}

static void migrateAppSettingsV4(const app_setting_v4 &legacy)
{
    parseAppSettings(default_app_setting);
    copySettingString(app_settings_save.weather_secret, sizeof(app_settings_save.weather_secret), legacy.weather_secret);
    copySettingString(app_settings_save.weather_city, sizeof(app_settings_save.weather_city), legacy.weather_city);
    copySettingString(app_settings_save.weather_provider, sizeof(app_settings_save.weather_provider), legacy.weather_provider);
    copySettingString(app_settings_save.weather_endpoint, sizeof(app_settings_save.weather_endpoint), legacy.weather_endpoint);
    copySettingString(app_settings_save.weather_endpoint_seniverse, sizeof(app_settings_save.weather_endpoint_seniverse), legacy.weather_endpoint_seniverse);
    copySettingString(app_settings_save.weather_endpoint_qweather, sizeof(app_settings_save.weather_endpoint_qweather), legacy.weather_endpoint_qweather);
    copySettingString(app_settings_save.weather_endpoint_aliyun_72158, sizeof(app_settings_save.weather_endpoint_aliyun_72158), legacy.weather_endpoint_aliyun_72158);
    copySettingString(app_settings_save.weather_endpoint_aliyun_10812, sizeof(app_settings_save.weather_endpoint_aliyun_10812), legacy.weather_endpoint_aliyun_10812);
    copySettingString(app_settings_save.weather_endpoint_aliyun_50139, sizeof(app_settings_save.weather_endpoint_aliyun_50139), legacy.weather_endpoint_aliyun_50139);
    copySettingString(app_settings_save.weather_endpoint_aliyun_71988, sizeof(app_settings_save.weather_endpoint_aliyun_71988), legacy.weather_endpoint_aliyun_71988);
    copySettingString(app_settings_save.weather_lat, sizeof(app_settings_save.weather_lat), legacy.weather_lat);
    copySettingString(app_settings_save.weather_lon, sizeof(app_settings_save.weather_lon), legacy.weather_lon);
    copySettingString(app_settings_save.weather_key_seniverse, sizeof(app_settings_save.weather_key_seniverse), legacy.weather_key_seniverse);
    copySettingString(app_settings_save.weather_key_qweather, sizeof(app_settings_save.weather_key_qweather), legacy.weather_key_qweather);
    copySettingString(app_settings_save.weather_key_aliyun_72158, sizeof(app_settings_save.weather_key_aliyun_72158), legacy.weather_key_aliyun_72158);
    copySettingString(app_settings_save.weather_key_aliyun_10812, sizeof(app_settings_save.weather_key_aliyun_10812), legacy.weather_key_aliyun_10812);
    copySettingString(app_settings_save.weather_key_aliyun_50139, sizeof(app_settings_save.weather_key_aliyun_50139), legacy.weather_key_aliyun_50139);
    copySettingString(app_settings_save.weather_key_aliyun_71988, sizeof(app_settings_save.weather_key_aliyun_71988), legacy.weather_key_aliyun_71988);
    fillWeatherDefaultsIfMissing();
}

void parseAppSettings(const char *input)
{
    cJSON *root = cJSON_Parse(input);
    if (root == NULL || !cJSON_IsObject(root))
    {
        ESP_LOGE("APP", "JSON解析失败");
        cJSON_Delete(root);
        root = cJSON_Parse(default_app_setting);
    }

    const char *city = jsonStringValue(root, "city");
    if (city != NULL && strlen(city) > 0)
        copySettingString(app_settings_save.weather_city, sizeof(app_settings_save.weather_city), city);

    const char *weather_provider = jsonStringValue(root, "weather_provider");
    if (weather_provider != NULL && strlen(weather_provider) > 0)
        copySettingString(app_settings_save.weather_provider, sizeof(app_settings_save.weather_provider), weather_provider);

    parseWeatherEndpointMap(root);
    parseWeatherEndpointField(root, "seniverse", "weather_endpoint_seniverse");
    parseWeatherEndpointField(root, "qweather", "weather_endpoint_qweather");
    parseWeatherEndpointField(root, "aliyun_72158", "weather_endpoint_aliyun_72158");
    parseWeatherEndpointField(root, "aliyun_10812", "weather_endpoint_aliyun_10812");
    parseWeatherEndpointField(root, "aliyun_50139", "weather_endpoint_aliyun_50139");
    parseWeatherEndpointField(root, "aliyun_71988", "weather_endpoint_aliyun_71988");

    const char *weather_endpoint = jsonStringValue(root, "weather_endpoint");
    if (weather_endpoint != NULL && strlen(weather_endpoint) > 0)
        saveActiveWeatherEndpointForProvider(app_settings_save.weather_provider, weather_endpoint);

    const char *weather_lat = jsonStringValue(root, "weather_lat");
    if (weather_lat != NULL)
        copySettingString(app_settings_save.weather_lat, sizeof(app_settings_save.weather_lat), weather_lat);

    const char *weather_lon = jsonStringValue(root, "weather_lon");
    if (weather_lon != NULL)
        copySettingString(app_settings_save.weather_lon, sizeof(app_settings_save.weather_lon), weather_lon);

    const char *seniverse_publickey = jsonStringValue(root, "weather_publickey_seniverse");
    if (seniverse_publickey == NULL)
        seniverse_publickey = jsonStringValue(root, "weather_publickey");
    if (seniverse_publickey != NULL)
        copySettingString(app_settings_save.weather_publickey_seniverse, sizeof(app_settings_save.weather_publickey_seniverse), seniverse_publickey);

    parseWeatherKeyMap(root);
    parseWeatherKeyField(root, "seniverse", "weather_key_seniverse");
    parseWeatherKeyField(root, "qweather", "weather_key_qweather");
    parseWeatherKeyField(root, "aliyun_72158", "weather_key_aliyun_72158");
    parseWeatherKeyField(root, "aliyun_10812", "weather_key_aliyun_10812");
    parseWeatherKeyField(root, "aliyun_50139", "weather_key_aliyun_50139");
    parseWeatherKeyField(root, "aliyun_71988", "weather_key_aliyun_71988");
    parseAliyunCredentialMap(root);
    parseAliyunCredentialFields(root, "aliyun_72158");
    parseAliyunCredentialFields(root, "aliyun_10812");
    parseAliyunCredentialFields(root, "aliyun_50139");
    parseAliyunCredentialFields(root, "aliyun_71988");

    const char *key_seniverse = jsonStringValue(root, "weather_key_seniverse");
    if (key_seniverse != NULL && strlen(key_seniverse) > 0)
        copySettingString(app_settings_save.weather_key_seniverse, sizeof(app_settings_save.weather_key_seniverse), key_seniverse);

    const char *key_qweather = jsonStringValue(root, "weather_key_qweather");
    if (key_qweather != NULL && strlen(key_qweather) > 0)
        copySettingString(app_settings_save.weather_key_qweather, sizeof(app_settings_save.weather_key_qweather), key_qweather);

    const char *key_aliyun_72158 = jsonStringValue(root, "weather_key_aliyun_72158");
    if (key_aliyun_72158 != NULL && strlen(key_aliyun_72158) > 0)
        copySettingString(app_settings_save.weather_key_aliyun_72158, sizeof(app_settings_save.weather_key_aliyun_72158), key_aliyun_72158);

    const char *key_aliyun_10812 = jsonStringValue(root, "weather_key_aliyun_10812");
    if (key_aliyun_10812 != NULL && strlen(key_aliyun_10812) > 0)
        copySettingString(app_settings_save.weather_key_aliyun_10812, sizeof(app_settings_save.weather_key_aliyun_10812), key_aliyun_10812);

    const char *key_aliyun_50139 = jsonStringValue(root, "weather_key_aliyun_50139");
    if (key_aliyun_50139 != NULL && strlen(key_aliyun_50139) > 0)
        copySettingString(app_settings_save.weather_key_aliyun_50139, sizeof(app_settings_save.weather_key_aliyun_50139), key_aliyun_50139);

    const char *key_aliyun_71988 = jsonStringValue(root, "weather_key_aliyun_71988");
    if (key_aliyun_71988 != NULL && strlen(key_aliyun_71988) > 0)
        copySettingString(app_settings_save.weather_key_aliyun_71988, sizeof(app_settings_save.weather_key_aliyun_71988), key_aliyun_71988);

    const char *weather = jsonStringValue(root, "weather");
    if (weather != NULL)
        saveActiveWeatherKeyForProvider(app_settings_save.weather_provider, weather);

    if (isAliyunWeatherProviderName(app_settings_save.weather_provider))
    {
        const char *weather_appcode = jsonStringValue(root, "weather_appcode");
        if (weather_appcode != NULL)
            saveActiveWeatherKeyForProvider(app_settings_save.weather_provider, weather_appcode);
        const char *weather_appkey = jsonStringValue(root, "weather_appkey");
        if (weather_appkey != NULL)
            saveAliyunAppKeyForProvider(app_settings_save.weather_provider, weather_appkey);
        const char *weather_appsecret = jsonStringValue(root, "weather_appsecret");
        if (weather_appsecret != NULL)
            saveAliyunAppSecretForProvider(app_settings_save.weather_provider, weather_appsecret);
    }

    fillWeatherDefaultsIfMissing();
    cJSON_Delete(root);
}

static void addAliyunCredentialJson(cJSON *parent, const char *provider)
{
    if (parent == NULL || !isAliyunWeatherProviderName(provider))
        return;
    cJSON *item = cJSON_AddObjectToObject(parent, provider);
    if (item == NULL)
        return;
    cJSON_AddStringToObject(item, "appcode", constWeatherKeySlotForProvider(provider));
    cJSON_AddStringToObject(item, "appkey", constWeatherAppKeySlotForProvider(provider));
    cJSON_AddStringToObject(item, "appsecret", constWeatherAppSecretSlotForProvider(provider));
    cJSON_AddBoolToObject(item, "configured", isProviderKeyConfigured(provider));
}

void appSettingsToJson(char *result)
{
    cJSON *item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "weather_provider", app_settings_save.weather_provider);
    cJSON_AddStringToObject(item, "weather_endpoint", app_settings_save.weather_endpoint);
    cJSON_AddStringToObject(item, "weather_endpoint_seniverse", constWeatherEndpointSlotForProvider("seniverse"));
    cJSON_AddStringToObject(item, "weather_endpoint_qweather", constWeatherEndpointSlotForProvider("qweather"));
    cJSON_AddStringToObject(item, "weather_endpoint_aliyun_72158", constWeatherEndpointSlotForProvider("aliyun_72158"));
    cJSON_AddStringToObject(item, "weather_endpoint_aliyun_10812", constWeatherEndpointSlotForProvider("aliyun_10812"));
    cJSON_AddStringToObject(item, "weather_endpoint_aliyun_50139", constWeatherEndpointSlotForProvider("aliyun_50139"));
    cJSON_AddStringToObject(item, "weather_endpoint_aliyun_71988", constWeatherEndpointSlotForProvider("aliyun_71988"));
    cJSON *endpoints = cJSON_AddObjectToObject(item, "weather_provider_endpoints");
    if (endpoints != NULL)
    {
        cJSON_AddStringToObject(endpoints, "seniverse", constWeatherEndpointSlotForProvider("seniverse"));
        cJSON_AddStringToObject(endpoints, "qweather", constWeatherEndpointSlotForProvider("qweather"));
        cJSON_AddStringToObject(endpoints, "aliyun_72158", constWeatherEndpointSlotForProvider("aliyun_72158"));
        cJSON_AddStringToObject(endpoints, "aliyun_10812", constWeatherEndpointSlotForProvider("aliyun_10812"));
        cJSON_AddStringToObject(endpoints, "aliyun_50139", constWeatherEndpointSlotForProvider("aliyun_50139"));
        cJSON_AddStringToObject(endpoints, "aliyun_71988", constWeatherEndpointSlotForProvider("aliyun_71988"));
    }
    cJSON_AddStringToObject(item, "weather_lat", app_settings_save.weather_lat);
    cJSON_AddStringToObject(item, "weather_lon", app_settings_save.weather_lon);
    cJSON_AddStringToObject(item, "weather", constWeatherKeySlotForProvider(app_settings_save.weather_provider));
    cJSON_AddStringToObject(item, "weather_key_seniverse", constWeatherKeySlotForProvider("seniverse"));
    cJSON_AddStringToObject(item, "weather_publickey_seniverse", app_settings_save.weather_publickey_seniverse);
    cJSON_AddStringToObject(item, "weather_publickey", strcmp(app_settings_save.weather_provider, "seniverse") == 0 ? app_settings_save.weather_publickey_seniverse : "");
    cJSON_AddStringToObject(item, "weather_key_qweather", constWeatherKeySlotForProvider("qweather"));
    cJSON_AddStringToObject(item, "weather_key_aliyun_72158", constWeatherKeySlotForProvider("aliyun_72158"));
    cJSON_AddStringToObject(item, "weather_key_aliyun_10812", constWeatherKeySlotForProvider("aliyun_10812"));
    cJSON_AddStringToObject(item, "weather_key_aliyun_50139", constWeatherKeySlotForProvider("aliyun_50139"));
    cJSON_AddStringToObject(item, "weather_key_aliyun_71988", constWeatherKeySlotForProvider("aliyun_71988"));
    cJSON_AddStringToObject(item, "weather_appcode_aliyun_72158", constWeatherKeySlotForProvider("aliyun_72158"));
    cJSON_AddStringToObject(item, "weather_appcode_aliyun_10812", constWeatherKeySlotForProvider("aliyun_10812"));
    cJSON_AddStringToObject(item, "weather_appcode_aliyun_50139", constWeatherKeySlotForProvider("aliyun_50139"));
    cJSON_AddStringToObject(item, "weather_appcode_aliyun_71988", constWeatherKeySlotForProvider("aliyun_71988"));
    cJSON_AddStringToObject(item, "weather_appkey_aliyun_72158", constWeatherAppKeySlotForProvider("aliyun_72158"));
    cJSON_AddStringToObject(item, "weather_appkey_aliyun_10812", constWeatherAppKeySlotForProvider("aliyun_10812"));
    cJSON_AddStringToObject(item, "weather_appkey_aliyun_50139", constWeatherAppKeySlotForProvider("aliyun_50139"));
    cJSON_AddStringToObject(item, "weather_appkey_aliyun_71988", constWeatherAppKeySlotForProvider("aliyun_71988"));
    cJSON_AddStringToObject(item, "weather_appsecret_aliyun_72158", constWeatherAppSecretSlotForProvider("aliyun_72158"));
    cJSON_AddStringToObject(item, "weather_appsecret_aliyun_10812", constWeatherAppSecretSlotForProvider("aliyun_10812"));
    cJSON_AddStringToObject(item, "weather_appsecret_aliyun_50139", constWeatherAppSecretSlotForProvider("aliyun_50139"));
    cJSON_AddStringToObject(item, "weather_appsecret_aliyun_71988", constWeatherAppSecretSlotForProvider("aliyun_71988"));
    cJSON_AddStringToObject(item, "weather_appcode", isAliyunWeatherProviderName(app_settings_save.weather_provider) ? constWeatherKeySlotForProvider(app_settings_save.weather_provider) : "");
    cJSON_AddStringToObject(item, "weather_appkey", constWeatherAppKeySlotForProvider(app_settings_save.weather_provider));
    cJSON_AddStringToObject(item, "weather_appsecret", constWeatherAppSecretSlotForProvider(app_settings_save.weather_provider));
    cJSON *key_values = cJSON_AddObjectToObject(item, "weather_provider_key_values");
    if (key_values != NULL)
    {
        cJSON_AddStringToObject(key_values, "seniverse", constWeatherKeySlotForProvider("seniverse"));
        cJSON_AddStringToObject(key_values, "qweather", constWeatherKeySlotForProvider("qweather"));
        cJSON_AddStringToObject(key_values, "aliyun_72158", constWeatherKeySlotForProvider("aliyun_72158"));
        cJSON_AddStringToObject(key_values, "aliyun_10812", constWeatherKeySlotForProvider("aliyun_10812"));
        cJSON_AddStringToObject(key_values, "aliyun_50139", constWeatherKeySlotForProvider("aliyun_50139"));
        cJSON_AddStringToObject(key_values, "aliyun_71988", constWeatherKeySlotForProvider("aliyun_71988"));
    }
    cJSON *credentials = cJSON_AddObjectToObject(item, "weather_provider_credentials");
    if (credentials != NULL)
    {
        addAliyunCredentialJson(credentials, "aliyun_72158");
        addAliyunCredentialJson(credentials, "aliyun_10812");
        addAliyunCredentialJson(credentials, "aliyun_50139");
        addAliyunCredentialJson(credentials, "aliyun_71988");
    }
    cJSON_AddBoolToObject(item, "weather_configured", isProviderKeyConfigured(app_settings_save.weather_provider));
    cJSON *keys = cJSON_AddObjectToObject(item, "weather_provider_keys");
    if (keys != NULL)
    {
        cJSON_AddBoolToObject(keys, "seniverse", isProviderKeyConfigured("seniverse"));
        cJSON_AddBoolToObject(keys, "qweather", isProviderKeyConfigured("qweather"));
        cJSON_AddBoolToObject(keys, "aliyun_72158", isProviderKeyConfigured("aliyun_72158"));
        cJSON_AddBoolToObject(keys, "aliyun_10812", isProviderKeyConfigured("aliyun_10812"));
        cJSON_AddBoolToObject(keys, "aliyun_50139", isProviderKeyConfigured("aliyun_50139"));
        cJSON_AddBoolToObject(keys, "aliyun_71988", isProviderKeyConfigured("aliyun_71988"));
    }
    cJSON_AddStringToObject(item, "city", app_settings_save.weather_city);
    writeJsonToBuffer(item, result, sizeof(jsonbuffer));
    cJSON_Delete(item);
}

void HAL::saveAppSettings()
{
    sanitizeAppSettingStrings(app_settings_save);
    if (!writeCurrentAppSettings(app_settings_save))
        ESP_LOGE("HAL", "Failed to persist application settings");
}

void HAL::loadAppSettings()
{
    File file = LittleFS.open("/.cfg.bin", "r");
    if (file)
    {
        size_t fileSize = file.size();
        if (fileSize == sizeof(app_settings_file_header) + sizeof(app_settings_save))
        {
            app_setting &loaded = app_settings_scratch.current;
            if (readCurrentAppSettings(file, loaded))
            {
                file.close();
                app_settings_save = loaded;
                fillWeatherDefaultsIfMissing();
                return;
            }
            file.close();
            goto reset;
        }
        if (fileSize == sizeof(app_settings_save))
        {
            app_setting &legacy = app_settings_scratch.current;
            memset(&legacy, 0, sizeof(legacy));
            int sz = file.read((uint8_t *)&legacy, sizeof(legacy));
            if (sz != sizeof(app_settings_save))
            {
                file.close();
                goto reset;
            }
            if (file.read() != -1)
            {
                file.close();
                goto reset;
            }
            file.close();
            sanitizeAppSettingStrings(legacy);
            app_settings_save = legacy;
            fillWeatherDefaultsIfMissing();
            saveAppSettings();
            return;
        }
        if (fileSize == sizeof(app_setting_v5))
        {
            app_setting_v5 &legacy = app_settings_scratch.v5;
            memset(&legacy, 0, sizeof(legacy));
            int sz = file.read((uint8_t *)&legacy, sizeof(legacy));
            file.close();
            if (sz != sizeof(legacy))
                goto reset;
            sanitizeAppSettingStrings(legacy);
            migrateAppSettingsV5(legacy);
            saveAppSettings();
            return;
        }
        if (fileSize == sizeof(app_setting_v4))
        {
            app_setting_v4 &legacy = app_settings_scratch.v4;
            memset(&legacy, 0, sizeof(legacy));
            int sz = file.read((uint8_t *)&legacy, sizeof(legacy));
            file.close();
            if (sz != sizeof(legacy))
                goto reset;
            sanitizeAppSettingStrings(legacy);
            migrateAppSettingsV4(legacy);
            saveAppSettings();
            return;
        }
        if (fileSize == sizeof(app_setting_v3))
        {
            app_setting_v3 &legacy = app_settings_scratch.v3;
            memset(&legacy, 0, sizeof(legacy));
            int sz = file.read((uint8_t *)&legacy, sizeof(legacy));
            file.close();
            if (sz != sizeof(legacy))
                goto reset;
            sanitizeAppSettingStrings(legacy);
            migrateAppSettingsV3(legacy);
            saveAppSettings();
            return;
        }
        if (fileSize == sizeof(app_setting_v2))
        {
            app_setting_v2 &legacy = app_settings_scratch.v2;
            memset(&legacy, 0, sizeof(legacy));
            int sz = file.read((uint8_t *)&legacy, sizeof(legacy));
            file.close();
            if (sz != sizeof(legacy))
                goto reset;
            sanitizeAppSettingStrings(legacy);
            migrateAppSettingsV2(legacy);
            saveAppSettings();
            return;
        }
        if (fileSize == sizeof(extended_app_setting))
        {
            extended_app_setting &legacy = app_settings_scratch.extended;
            memset(&legacy, 0, sizeof(legacy));
            int sz = file.read((uint8_t *)&legacy, sizeof(legacy));
            file.close();
            if (sz != sizeof(legacy))
                goto reset;
            sanitizeAppSettingStrings(legacy);
            migrateExtendedAppSettings(legacy);
            saveAppSettings();
            return;
        }
        if (fileSize == sizeof(legacy_app_setting))
        {
            legacy_app_setting &legacy = app_settings_scratch.legacy;
            memset(&legacy, 0, sizeof(legacy));
            int sz = file.read((uint8_t *)&legacy, sizeof(legacy));
            file.close();
            if (sz != sizeof(legacy))
                goto reset;
            sanitizeAppSettingStrings(legacy);
            migrateLegacyAppSettings(legacy);
            saveAppSettings();
            return;
        }
        file.close();
        goto reset;
    }
    else
    {
    reset:
        memset(&app_settings_save, 0, sizeof(app_settings_save));
        parseAppSettings(default_app_setting);
        saveAppSettings();
    }
}
void handleJson()
{
    if (server.method() == HTTP_POST && server.hasArg("plain"))
    {
        cJSON *root = cJSON_Parse(server.arg("plain").c_str());
        if (root == NULL || !cJSON_IsObject(root)) {
            cJSON_Delete(root);
            server.send(500, "text/plain", "ERR 500");
            return;
        }
        cJSON_Delete(root);
        parseAppSettings(server.arg("plain").c_str());
        hal.saveAppSettings();
        hal.weather_update = false;
        server.send(200, "text/plain", "OK");
        return;
    }
    else if (server.method() == HTTP_GET)
    {
        appSettingsToJson(jsonbuffer);
        server.send(200, "application/json", jsonbuffer);
        return;
    }
    server.send(500, "text/plain", "ERR 500");
}

extern bool testWeatherProvider(const char *provider, const char *endpoint, const char *key, const char *appKey, const char *appSecret, const char *publicKey, const char *location, const char *lat, const char *lon, char *result, size_t result_size);

void handleWeatherTest()
{
    if (server.method() != HTTP_POST || !server.hasArg("plain"))
    {
        server.send(400, "application/json", "{\"ok\":false,\"message\":\"Missing JSON body\"}");
        return;
    }

    cJSON *root = cJSON_Parse(server.arg("plain").c_str());
    if (root == NULL || !cJSON_IsObject(root))
    {
        cJSON_Delete(root);
        server.send(400, "application/json", "{\"ok\":false,\"message\":\"Invalid JSON\"}");
        return;
    }

    const char *provider = jsonStringValue(root, "weather_provider");
    if (provider == NULL || strlen(provider) == 0)
        provider = app_settings_save.weather_provider;
    const char *endpoint = jsonStringValue(root, "weather_endpoint");
    if (endpoint == NULL || strlen(endpoint) == 0)
        endpoint = constWeatherEndpointSlotForProvider(provider);
    const char *key = jsonStringValue(root, "weather");
    if (key == NULL)
        key = constWeatherKeySlotForProvider(provider);
    const char *appCode = jsonStringValue(root, "weather_appcode");
    if (isAliyunWeatherProviderName(provider) && appCode != NULL)
        key = appCode;
    const char *appKey = jsonStringValue(root, "weather_appkey");
    if (appKey == NULL)
        appKey = constWeatherAppKeySlotForProvider(provider);
    const char *appSecret = jsonStringValue(root, "weather_appsecret");
    if (appSecret == NULL)
        appSecret = constWeatherAppSecretSlotForProvider(provider);
    const char *publicKey = jsonStringValue(root, "weather_publickey");
    if (publicKey == NULL)
        publicKey = app_settings_save.weather_publickey_seniverse;
    const char *location = jsonStringValue(root, "city");
    if (location == NULL || strlen(location) == 0)
        location = app_settings_save.weather_city;
    const char *lat = jsonStringValue(root, "weather_lat");
    if (lat == NULL)
        lat = app_settings_save.weather_lat;
    const char *lon = jsonStringValue(root, "weather_lon");
    if (lon == NULL)
        lon = app_settings_save.weather_lon;

    UBaseType_t stackBefore = uxTaskGetStackHighWaterMark(NULL);
    ESP_LOGI("WEATHER_TEST", "begin stack_free=%u internal_heap=%" PRIu32, (unsigned)stackBefore, esp_get_free_internal_heap_size());
    char message[192];
    bool ok = testWeatherProvider(provider, endpoint, key, appKey, appSecret, publicKey, location, lat, lon, message, sizeof(message));
    UBaseType_t stackAfter = uxTaskGetStackHighWaterMark(NULL);
    ESP_LOGI("WEATHER_TEST", "end ok=%d stack_free=%u internal_heap=%" PRIu32, ok, (unsigned)stackAfter, esp_get_free_internal_heap_size());
    cJSON *json = cJSON_CreateObject();
    cJSON_AddBoolToObject(json, "ok", ok);
    cJSON_AddStringToObject(json, "provider", provider);
    cJSON_AddStringToObject(json, "endpoint", endpoint);
    cJSON_AddStringToObject(json, "message", message);
    writeJsonToBuffer(json, jsonbuffer, sizeof(jsonbuffer));
    cJSON_Delete(json);
    cJSON_Delete(root);
    server.send(ok ? 200 : 502, "application/json", jsonbuffer);
}

#include "webserver/favicon.h"
#include "webserver/index.h"
#include "webserver/js.h"
#include "webserver/ffmpeg.h"
#include "webserver/css.h"
#include "webserver/menu.h"
#include "webserver/close_eye.h"
#include "webserver/i.h"
#include "webserver/local.h"
#include "webserver/open_eye.h"
#include "webserver/wifi.h"
#include "webserver/arrow_left.h"
#include "webserver/arrow_right.h"
#include "webserver/check.h"
#include "webserver/check2.h"
#include "webserver/demo.h"
#include "webserver/error_bg.h"
#include "webserver/error_m.h"
#include "webserver/ic_d.h"
#include "webserver/ic_del.h"
#include "webserver/image2.h"
#include "webserver/keytone.h"
#include "webserver/keytone2.h"
#include "webserver/nothing.h"
#include "webserver/play.h"
#include "webserver/setting2.h"
#include "webserver/spead.h"
#include "webserver/stop.h"
#include "webserver/theme1.h"
#include "webserver/theme2.h"
#include "webserver/theme3.h"
#include "webserver/theme4.h"
#include "webserver/time_bg.h"
#include "webserver/time_ic.h"
#include "webserver/video2.h"
#include "webserver/weather.h"
#include "webserver/worker.h"

void HAL::start_webserver()
{
    webserver_should_run = true;
    if (webserver_task != NULL)
        return;

    if (WiFi.getMode() == WIFI_OFF || (WiFi.getMode() == WIFI_STA && WiFi.isConnected() == false))
    {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("SKYLOONG 4.0 Screen");
    }
    else if (WiFi.getMode() == WIFI_STA)
    {
        WiFi.setAutoReconnect(true);
    }
    WiFi.setSleep(false);

    MDNS.begin("SKYLOONG 4.0 Screen");
    server.enableCORS(true);

    if (WiFi.getMode() == WIFI_AP) {
        dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
        server.onNotFound([]() {
            server.sendHeader("Location", "http://192.168.4.1/wifi");
            server.send(302, "text/plain", "redirect to captive portal");
        });
    } else {
        server.onNotFound([]() {
            if (!handleFileRead(server.uri())) {
            server.send(404, "text/plain", "FileNotFound");
            }
        });
    }

    if (!webserver_routes_registered) {
    server.on("/info", HTTP_GET, []() {
        cJSON *json = cJSON_CreateObject();

        if (WiFi.getMode() == WIFI_AP) {
            cJSON_AddStringToObject(json, "mode", "AP");
            cJSON_AddStringToObject(json, "ssid", "SKYLOONG 4.0 Screen");
            cJSON_AddStringToObject(json, "ip", "192.168.4.1");
        } else {
            cJSON_AddStringToObject(json, "mode", "STA");
            cJSON_AddStringToObject(json, "ssid", WiFi.SSID().c_str());
            cJSON_AddStringToObject(json, "ip", WiFi.localIP().toString().c_str());
        }

        cJSON_AddNumberToObject(json, "theme", hal.config_theme);
        cJSON_AddNumberToObject(json, "space_breath_speed", hal.config_space_breath_speed);
        cJSON_AddBoolToObject(json, "aps_enable", hal.aps_enable);
        cJSON_AddBoolToObject(json, "weather_enable", hal.weather_enable);
        cJSON_AddBoolToObject(json, "gif_enable", hal.gif_enable);
        cJSON_AddBoolToObject(json, "jpg_enable", hal.jpg_enable);
        cJSON_AddBoolToObject(json, "wifi_connected", WiFi.status() == WL_CONNECTED);
        cJSON_AddNumberToObject(json, "wifi_saved_count", WiFiMgr.count());
        cJSON_AddNumberToObject(json, "screen_width", screenWidth);
        cJSON_AddNumberToObject(json, "screen_height", screenHeight);
        cJSON_AddStringToObject(json, "video_fit", hal.config_video_fit);
        cJSON_AddBoolToObject(json, "video_audio", hal.config_video_audio);
        cJSON_AddNumberToObject(json, "time_roll", hal.config_time_roll);
        cJSON_AddStringToObject(json, "jpg_mode", hal.config_jpg_mode);
        cJSON_AddStringToObject(json, "jpg_file", hal.config_jpg_file);
        long timezone = i18n::getNTPOffset();
        timezone /= 3600;
        cJSON_AddNumberToObject(json, "timezone", timezone);
        cJSON_AddNumberToObject(json, "language", i18n::getLanguage());
        cJSON_AddNumberToObject(json, "keytone", hal.config_keytone);
        cJSON_AddStringToObject(json, "keytone_file", hal.config_keytone_file);
        cJSON_AddNumberToObject(json, "volume", hal._volume);
        cJSON_AddNumberToObject(json, "heap_free", ESP.getFreeHeap());
        cJSON_AddNumberToObject(json, "webserver_uptime_ms", hal.webserver_last_alive_ms);
        cJSON_AddNumberToObject(json, "webserver_stack_free", hal.webserver_task != NULL ? uxTaskGetStackHighWaterMark(hal.webserver_task) : 0);

        writeJsonToBuffer(json, jsonbuffer, sizeof(jsonbuffer));
        cJSON_Delete(json);
        server.send(200, "application/json", jsonbuffer);
    });

    server.on("/health", HTTP_GET, []() {
        cJSON *json = cJSON_CreateObject();
        cJSON_AddBoolToObject(json, "ok", hal.server_started);
        cJSON_AddStringToObject(json, "mode", WiFi.getMode() == WIFI_AP ? "AP" : "STA");
        cJSON_AddStringToObject(json, "ip", WiFi.getMode() == WIFI_AP ? WiFi.softAPIP().toString().c_str() : WiFi.localIP().toString().c_str());
        cJSON_AddBoolToObject(json, "wifi_connected", WiFi.status() == WL_CONNECTED);
        addHeapDiagnostics(json);
        cJSON_AddNumberToObject(json, "uptime_ms", millis());
        cJSON_AddNumberToObject(json, "webserver_alive_ms", hal.webserver_last_alive_ms);
        cJSON_AddNumberToObject(json, "webserver_stack_free", hal.webserver_task != NULL ? uxTaskGetStackHighWaterMark(hal.webserver_task) : 0);
        cJSON_AddNumberToObject(json, "current_app", appManagerLite.currentApp != NULL ? appManagerLite.currentApp->appid : 0);
        writeJsonToBuffer(json, jsonbuffer, sizeof(jsonbuffer));
        cJSON_Delete(json);
        server.send(200, "application/json", jsonbuffer);
    });

    server.on("/scan_networks", HTTP_GET, []() {
        wifi_mode_t mode = WiFi.getMode();
        int8_t result = WiFi.scanNetworks(false, false);
        if (mode == WIFI_AP) {
            WiFi.mode(WIFI_AP);
        }

        cJSON *json = cJSON_CreateObject();
        cJSON *array = cJSON_AddArrayToObject(json, "networks");

        if (result >= 0) {
            for (int i = 0; i < result; ++i) {
                cJSON *item = cJSON_CreateObject();
                cJSON_AddStringToObject(item, "ssid", WiFi.SSID(i).c_str());
                cJSON_AddNumberToObject(item, "rssi", WiFi.RSSI(i));
                cJSON_AddItemToArray(array, item);
            }
        }

        writeJsonToBuffer(json, jsonbuffer, sizeof(jsonbuffer));
        cJSON_Delete(json);
        server.send(200, "application/json", jsonbuffer);
    });

    server.on("/config_wifi", HTTP_POST, []() {
        if (server.hasArg("ssid")) {
            String ssid = server.arg("ssid");
            String password = server.hasArg("password") ? server.arg("password") : "";
            ssid.trim();
            if (ssid.length() == 0) {
                server.send(500, "text/plain", "ERR 500");
                return;
            }
            strncpy(hal.ssid, ssid.c_str(), sizeof(hal.ssid) - 1);
            hal.ssid[sizeof(hal.ssid) - 1] = '\0';
            strncpy(hal.password, password.c_str(), sizeof(hal.password) - 1);
            hal.password[sizeof(hal.password) - 1] = '\0';
            hal.config_wifi = true;
            server.send(200, "text/plain", "OK");
        } else {
            server.send(500, "text/plain", "ERR 500");
        }
    });

    server.on("/config_theme", HTTP_POST, []() {
        if (server.hasArg("theme")) {
            hal.config_theme = server.arg("theme").toInt();
            hal.pref.putInt("theme", hal.config_theme);
            if (server.hasArg("space_breath_speed")) {
                int speed = server.arg("space_breath_speed").toInt();
                if (speed < 0 || speed > 2) {
                    server.send(400, "text/plain", "ERR 400");
                    return;
                }
                hal.config_space_breath_speed = (uint8_t)speed;
                hal.pref.putUInt("space_breath", hal.config_space_breath_speed);
            }
            server.send(200, "text/plain", "OK");
        } else {
            server.send(500, "text/plain", "ERR 500");
        }
    });

    server.on("/config_timezone", HTTP_POST, []() {
        if (server.hasArg("timezone")) {
            long timezone = server.arg("timezone").toInt();
            if (timezone >= -12 && timezone <= 12) {
                timezone *= 3600;
                i18n::setNTPOffset(timezone);
                hal.pref.putInt("ntp", timezone);
                if(hal.NTPSync())
                    hal.time_sync = true;
            } else {
               server.send(500, "text/plain", "ERR 500"); 
            }

            server.send(200, "text/plain", "OK");
        } else {
            server.send(500, "text/plain", "ERR 500");
        }
    });

    server.on("/config_language", HTTP_POST, []() {
        if (server.hasArg("language")) {
            long language = server.arg("language").toInt();
            if (language == 0 || language == 1) {
                i18n::setLanguage(language);
                hal.pref.putUInt("lang", language);
                hal.lang_refresh = true;
            } else {
               server.send(500, "text/plain", "ERR 500"); 
            }

            server.send(200, "text/plain", "OK");
        } else {
            server.send(500, "text/plain", "ERR 500");
        }
    });

    server.on("/config_keytone", HTTP_POST, []() {
        if (server.hasArg("keytone") && server.hasArg("keytone_file")) {
            hal.config_keytone = server.arg("keytone").toInt();
            hal.pref.putInt("keytone", hal.config_keytone);
            strcpy(hal.config_keytone_file, server.arg("keytone_file").c_str());
            hal.pref.putString("keytone_file", hal.config_keytone_file);
            server.send(200, "text/plain", "OK");
        } else {
            server.send(500, "text/plain", "ERR 500");
        }
    });

    server.on("/config_volume", HTTP_POST, []() {
        if (server.hasArg("volume")) {
            int volume = server.arg("volume").toInt();
            if (volume >= 0 && volume <= 9) {
                hal.setVolume(volume);
                hal.pref.putUInt("volume", hal._volume);
                server.send(200, "text/plain", "OK");
            } else {
                server.send(500, "text/plain", "ERR 500");
            }
        } else {
            server.send(500, "text/plain", "ERR 500");
        }
    });

    server.on("/config_app_aps", HTTP_POST, []() {
        if (server.hasArg("enable")) {
            hal.aps_enable = server.arg("enable").toBool();
            hal.pref.putBool("aps_enable", hal.aps_enable);
            server.send(200, "text/plain", "OK");
        } else {
            server.send(500, "text/plain", "ERR 500");
        }
    });

    server.on("/config_app_gif", HTTP_POST, []() {
        if (server.hasArg("enable")) {
            hal.gif_enable = server.arg("enable").toBool();
            hal.pref.putBool("gif_enable", hal.gif_enable);
            if (server.hasArg("video_fit")) {
                const String video_fit = server.arg("video_fit");
                if (isVideoFitValueValid(video_fit)) {
                    setVideoFitConfig(video_fit);
                    hal.pref.putString("video_fit", hal.config_video_fit);
                } else {
                    server.send(500, "text/plain", "ERR 500");
                    return;
                }
            }
            if (server.hasArg("video_audio")) {
                hal.config_video_audio = server.arg("video_audio").toBool();
                hal.pref.putBool("video_audio", hal.config_video_audio);
            }
            server.send(200, "text/plain", "OK");
        } else {
            server.send(500, "text/plain", "ERR 500");
        }
    });

    server.on("/config_app_jpg", HTTP_POST, []() {
        if (server.hasArg("enable") && server.hasArg("jpg_mode") && server.hasArg("jpg_file") && server.hasArg("time_roll")) {
            hal.jpg_enable = server.arg("enable").toBool();
            hal.pref.putBool("jpg_enable", hal.jpg_enable);
            hal.config_time_roll = server.arg("time_roll").toInt();
            hal.pref.putInt("t_r", hal.config_time_roll);
            strcpy(hal.config_jpg_mode, server.arg("jpg_mode").c_str());
            hal.pref.putString("jpg_mode", hal.config_jpg_mode);
            strcpy(hal.config_jpg_file, server.arg("jpg_file").c_str());
            hal.pref.putString("jpg_file", hal.config_jpg_file);
            server.send(200, "text/plain", "OK");
        } else {
            server.send(500, "text/plain", "ERR 500");
        }
    });

    server.on("/config_app_weather", HTTP_POST, []() {
        if (server.hasArg("enable")) {
            hal.weather_enable = server.arg("enable").toBool();
            hal.pref.putBool("weather_enable", hal.weather_enable);
            server.send(200, "text/plain", "OK");
        } else {
            server.send(500, "text/plain", "ERR 500");
        }
    });

    server.on("/list", HTTP_GET, handleFileList);
    server.on("/edit", HTTP_PUT, handleFileCreate);    // create file
    server.on("/edit", HTTP_DELETE, handleFileDelete); // delete file
    server.on("/edit", HTTP_POST, []() {
        server.send(200, "text/plain", ""); 
    }, handleFileUpload);

    server.on("/time", HTTP_POST, handleTime);
    server.on("/reboot", HTTP_POST, []() {
        server.send(200, "text/plain", "OK");
        delay(100);
        ESP.restart(); 
    });
    server.on("/config.json", handleJson);
    server.on("/weather_test", HTTP_POST, handleWeatherTest);

    server.on("/favicon.ico", HTTP_GET, []() { 
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/x-icon", (const char *)__web_favicon_ico_gz, sizeof(__web_favicon_ico_gz));
    });
    server.on("/", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "text/html", (const char *)__web_index_html_gz, sizeof(__web_index_html_gz)); 
    });
    server.on("/wifi", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "text/html", (const char *)__web_index_html_gz, sizeof(__web_index_html_gz)); 
    });
    server.on("/index.js", HTTP_GET, []() {
        sendGzippedAsset("application/javascript", __web_index_js_gz, sizeof(__web_index_js_gz), false);
    });
    server.on("/ffmpeg.js", HTTP_GET, []() {
        sendGzippedAsset("application/javascript", __web_ffmpeg_js_gz, sizeof(__web_ffmpeg_js_gz));
    });
    server.on("/index.css", HTTP_GET, []() {
        sendGzippedAsset("text/css", __web_index_css_gz, sizeof(__web_index_css_gz), false);
    });

    server.on("/menu.jpg", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/jpeg", (const char *)__web_menu_jpg_gz, sizeof(__web_menu_jpg_gz));
    });
    server.on("/close_eye.svg", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/svg+xml", (const char *)__web_close_eye_svg_gz, sizeof(__web_close_eye_svg_gz));
    });
    server.on("/i.svg", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/svg+xml", (const char *)__web_i_svg_gz, sizeof(__web_i_svg_gz));
    });
    server.on("/local.svg", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/svg+xml", (const char *)__web_local_svg_gz, sizeof(__web_local_svg_gz));
    });
    server.on("/open_eye.svg", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/svg+xml", (const char *)__web_open_eye_svg_gz, sizeof(__web_open_eye_svg_gz));
    });
    server.on("/wifi.svg", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/svg+xml", (const char *)__web_wifi_svg_gz, sizeof(__web_wifi_svg_gz));
    });
    server.on("/arrow_left.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_arrow_left_png_gz, sizeof(__web_arrow_left_png_gz));
    });
    server.on("/arrow_right.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_arrow_right_png_gz, sizeof(__web_arrow_right_png_gz));
    });
    server.on("/check.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_check_png_gz, sizeof(__web_check_png_gz));
    });
    server.on("/check2.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_check2_png_gz, sizeof(__web_check2_png_gz));
    });
    server.on("/demo.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_demo_png_gz, sizeof(__web_demo_png_gz));
    });
    server.on("/error_bg.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_error_bg_png_gz, sizeof(__web_error_bg_png_gz));
    });
    server.on("/error_m.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_error_m_png_gz, sizeof(__web_error_m_png_gz));
    });
    server.on("/ic_d.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_ic_d_png_gz, sizeof(__web_ic_d_png_gz));
    });
    server.on("/ic_del.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_ic_del_png_gz, sizeof(__web_ic_del_png_gz));
    });
    server.on("/image2.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_image2_png_gz, sizeof(__web_image2_png_gz));
    });
    server.on("/keytone.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_keytone_png_gz, sizeof(__web_keytone_png_gz));
    });
    server.on("/keytone2.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_keytone2_png_gz, sizeof(__web_keytone2_png_gz));
    });
    server.on("/nothing.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_nothing_png_gz, sizeof(__web_nothing_png_gz));
    });
    server.on("/play.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_play_png_gz, sizeof(__web_play_png_gz));
    });
    server.on("/setting2.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_setting2_png_gz, sizeof(__web_setting2_png_gz));
    });
    server.on("/spead.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_spead_png_gz, sizeof(__web_spead_png_gz));
    });
    server.on("/stop.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_stop_png_gz, sizeof(__web_stop_png_gz));
    });
    server.on("/theme1.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_theme1_png_gz, sizeof(__web_theme1_png_gz));
    });
    server.on("/theme2.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_theme2_png_gz, sizeof(__web_theme2_png_gz));
    });
    server.on("/theme3.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_theme3_png_gz, sizeof(__web_theme3_png_gz));
    });
    server.on("/theme4.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_theme4_png_gz, sizeof(__web_theme4_png_gz));
    });
    server.on("/time_bg.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_time_bg_png_gz, sizeof(__web_time_bg_png_gz));
    });
    server.on("/time_ic__.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_time_ic___png_gz, sizeof(__web_time_ic___png_gz));
    });
    server.on("/video2.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_video2_png_gz, sizeof(__web_video2_png_gz));
    });
    server.on("/weather.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_weather_png_gz, sizeof(__web_weather_png_gz));
    });
    server.on("/assets/worker-224792ee.js", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "application/javascript", (const char *)__web_assets_worker_224792ee_js_gz, sizeof(__web_assets_worker_224792ee_js_gz));
    });
    webserver_routes_registered = true;
    }

    BaseType_t webserver_task_status = xTaskCreatePinnedToCore([](void *p)
    {
        while (1)
        {
            if(hal.server_started) {
                hal.webserver_last_alive_ms = millis();
                if (WiFi.getMode() == WIFI_STA && WiFi.status() != WL_CONNECTED) {
                    WiFi.reconnect();
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    continue;
                }
                server.handleClient();
            }
            vTaskDelay(5);
        }
    }, "webserver", 12288, NULL, 3, &webserver_task, 1);

    if (webserver_task_status != pdPASS)
    {
        ESP_LOGE("SERVER", "HTTP server task create failed, heap=%" PRIu32, esp_get_free_internal_heap_size());
        webserver_task = NULL;
        hal.server_started = false;
        return;
    }

    hal.server_started = true;
    hal.webserver_last_alive_ms = millis();
    server.begin();
    ESP_LOGI("SERVER", "HTTP server started");
    GUI::toast(_tr(I18N_ID_SERVER_STARTED));
}

void HAL::recover_webserver()
{
    bool should_run = webserver_should_run;
    ESP_LOGW("SERVER", "Recovering HTTP server, heap=%" PRIu32, esp_get_free_internal_heap_size());
    server.stop();
    if (webserver_task != NULL)
    {
        vTaskDelete(webserver_task);
        webserver_task = NULL;
    }
    server_started = false;
    webserver_last_alive_ms = 0;
    webserver_should_run = should_run;
    if (webserver_should_run)
        start_webserver();
}

void HAL::stop_webserver()
{
    webserver_should_run = false;
    if (webserver_task == NULL)
        return;
    server.stop();
    if (WiFi.getMode() == WIFI_AP)
    {
        WiFi.mode(WIFI_OFF);
    }
    hal.server_started = false;
    hal.webserver_last_alive_ms = 0;
    delay(50);
    vTaskDelete(webserver_task);
    webserver_task = NULL;
    GUI::toast(_tr(I18N_ID_SERVER_STOPPED));
}
extern bool stop_protocol;
void HAL::forceExitSettings()
{
    stop_protocol = true;
    hal.send_sysctl(EVENT_EXIT_SETTING);
}
