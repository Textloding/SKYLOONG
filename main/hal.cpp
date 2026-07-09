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
static char *TAG = "audio_init";

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

static bool playAudioFileFromLittleFS(const char *filename)
{
    if (filename == NULL || strlen(filename) == 0)
        return false;

    char path[64] = "/";
    strncat(path, filename, sizeof(path) - strlen(path) - 1);

    uint8_t *data = readFile(path);
    if (data == NULL)
        return false;

    const size_t len = sizeFile(path);
    bool played = false;
    if (hasFileSuffix(filename, "wav"))
    {
        i2s.playWAV(data, len);
        played = true;
    }
    else if (hasFileSuffix(filename, "mp3"))
    {
        i2s.playMP3(data, len);
        played = true;
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
    if (DS1302_isHalted(&hal.rtc))
    {
        DS1302_writeProtect(&hal.rtc, false);
        DS1302_halt(&hal.rtc, false);
        GUI::toast(_tr(I18N_ID_RTC_SHUTDOWN), true, 10000);
    }
    while (1)
    {
        sysctl_event_t event;
        xQueueReceive(hal._queue, &event, portMAX_DELAY);
        switch (event.type)
        {
        case EVENT_GET_TIME:
            DS1302_getDateTime(&hal.rtc, &hal.datetime);
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
                i2s.end();
                es8311_voice_volume_set(es_handle, 0, NULL);
                es8311_power_down(es_handle);
                digitalWrite(AUDIO_AMP_CTRL, LOW);

            }
            else
            {
                hal.setBrightness(hal._brightness);
                digitalWrite(AUDIO_AMP_CTRL, HIGH);
                hal.audio_init();
                hal.setVolume(hal._volume);
            }
            break;
        case EVENT_APM_CHANGED:
            hal.APM = event.data;
            hal.APMChanged = true;
            break;
        case EVENT_KB_KEYPRESS:
            if (hal.config_keytone == 1) {
                hal.keytone_play =  true;
                i2s.playWAV(__keytone_keytone1_wav, __keytone_keytone1_wav_len);
                hal.keytone_play =  false;
            } else if (hal.config_keytone == 2) {
                hal.keytone_play =  true;
                i2s.playWAV(__keytone_keytone2_wav, __keytone_keytone2_wav_len);
                hal.keytone_play =  false;
            } else if (hal.config_keytone == 3) {
                hal.keytone_play =  true;
                i2s.playWAV(__keytone_keytone3_wav, __keytone_keytone3_wav_len);
                hal.keytone_play =  false;
            } else if (hal.config_keytone == 4) {
                hal.keytone_play =  true;
                playAudioFileFromLittleFS(hal.config_keytone_file);
                hal.keytone_play =  false;
            } else if (hal.config_keytone == 5) {
                hal.keytone_play =  true;
                i2s.playWAV(__keytone_keytone4_wav, __keytone_keytone4_wav_len);
                hal.keytone_play =  false;
            } else if (hal.config_keytone == 6) {
                hal.keytone_play =  true;
                i2s.playWAV(__keytone_keytone5_wav, __keytone_keytone5_wav_len);
                hal.keytone_play =  false;
            } else if (hal.config_keytone == 7) {
                hal.keytone_play =  true;
                i2s.playWAV(__keytone_keytone6_wav, __keytone_keytone6_wav_len);
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
                    es8311_power_down(es_handle);
                    digitalWrite(AUDIO_AMP_CTRL, LOW);
                    hal.goSleep();
                    digitalWrite(AUDIO_AMP_CTRL, HIGH);
                    hal.audio_init();
                    hal.setVolume(hal._volume);   
                }
                else
                {
                    screen_is_sleep = true;
                    ledcWrite(PIN_DISPLAY_BL, 0);
                    audio_is_sleep = true;
                    i2s.end();
                    es8311_voice_volume_set(es_handle, 0, NULL);
                    es8311_power_down(es_handle);
                    digitalWrite(AUDIO_AMP_CTRL, LOW);
                    xSemaphoreGive(hal._mutex);
                }
            }
            else
            {
                ESP_LOGE("HAL", "无法获取锁");
                es8311_power_down(es_handle);
                digitalWrite(AUDIO_AMP_CTRL, LOW);
                hal.goSleep();
                digitalWrite(AUDIO_AMP_CTRL, HIGH);
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

esp_err_t HAL::audio_init()
{
    pinMode(AUDIO_AMP_CTRL, OUTPUT);
    digitalWrite(AUDIO_AMP_CTRL, HIGH);

    Wire.begin(AUDIO_CODEC_I2C_SDA_PIN, AUDIO_CODEC_I2C_SCL_PIN);

    ESP_RETURN_ON_FALSE(es_handle, ESP_FAIL, TAG, "es8311 create failed");
    const es8311_clock_config_t es_clk = {
        .mclk_inverted = false,
        .sclk_inverted = false,
        .mclk_from_mclk_pin = true,
        .mclk_frequency = AUDIO_MCLK_FREQ_HZ,
        .sample_frequency = AUDIO_SAMPLE_RATE
    };
    ESP_ERROR_CHECK(es8311_init(es_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16));
    ESP_RETURN_ON_ERROR(es8311_microphone_config(es_handle, false), TAG, "set es8311 microphone failed");

    i2s.setPins(AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN, AUDIO_I2S_GPIO_MCLK);
    if (!i2s.begin(I2S_MODE_STD, AUDIO_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
        ESP_LOGE("HAL", "Failed to initialize I2S bus!");
        return ESP_FAIL;
    }

    return ESP_OK;
}

void HAL::audio_stop()
{
    i2s.stop();
}

void HAL::init()
{
    memset(&datetime, 0, sizeof(DS1302_DateTime));
    WiFi.setHostname("SKYLOONG 4.0 Screen");
    lcd_init();

    hal.audio_init();

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

    screen_buf = (lv_color_t *)heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_SPIRAM);
    screen_buf2 = (lv_color_t *)heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_SPIRAM);

    lv_disp_draw_buf_init(&draw_buf, screen_buf, screen_buf2, DRAW_BUF_SIZE / 2);
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

    xTaskCreatePinnedToCore(task_systemctl, "task_systemctl", 8192, NULL, 3, NULL, 0);
}

void HAL::getTime()
{
    send_sysctl(EVENT_GET_TIME);
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
void HAL::setTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
    DS1302_DateTime dt;
    DS1302_writeProtect(&rtc, false);
    DS1302_halt(&rtc, false);
    dt.second = second;
    dt.minute = minute;
    dt.hour = hour;
    dt.dayWeek = getDoW(year, month, day);
    dt.dayMonth = day;
    dt.month = month;
    dt.year = year;
    DS1302_setDateTime(&rtc, &dt);
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
    time_t tm_now = 0;
    Udp.begin(8888);
    for (uint8_t i = 0; i < 5; ++i)
    {
        delay(10);
        tm_now = getNtpTime();
        if (tm_now != 0 && tm_now != 0xffffffff)
        {
            tm_now += i18n::getNTPOffset();
            tm time_now;
            localtime_r(&tm_now, &time_now);
            ESP_LOGI("NTP", "NTP同步成功: %d-%d-%d %d:%d:%d", time_now.tm_year + 1900, time_now.tm_mon + 1, time_now.tm_mday, time_now.tm_hour, time_now.tm_min, time_now.tm_sec);
            setTime(time_now.tm_year + 1900, time_now.tm_mon + 1, time_now.tm_mday, time_now.tm_hour, time_now.tm_min, time_now.tm_sec);
            break;
        }
        else
        {
            tm_now = 0;
        }
    }
    Udp.stop();
    if (tm_now == 0)
        return false;
    return true;
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
    static uint8_t volume_lut[10] = {0, 38, 46, 53, 59, 64, 68, 71, 73, 74};
    if (volume >= 0 && volume <= 9)
    {
        _volume = volume;
        es8311_voice_volume_set(es_handle, volume_lut[volume], NULL);
    }
    else
    {
        ESP_LOGE("HAL", "音量设置错误: %d", volume);
        es8311_voice_volume_set(es_handle, AUDIO_VOICE_VOLUME, NULL);
    }
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

static void sendGzippedAsset(const char *contentType, const uint8_t *content, size_t contentLength)
{
    server.sendHeader("Content-Encoding", "gzip", true);
    server.sendHeader("Cache-Control", "public, max-age=31536000, immutable");
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
        timeval tv;
        tm newtime;
        tv.tv_sec = atol(time.c_str());
        tv.tv_usec = 0;
        if (tv.tv_sec > 0)
        {
            tv.tv_sec += i18n::getNTPOffset();
            localtime_r(&tv.tv_sec, &newtime);
            hal.setTime(newtime.tm_year + 1900, newtime.tm_mon + 1, newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec);
            server.send(200, "text/plain", "OK");
            return;
        }
    }
    server.send(500, "text/plain", "ERR 500");
}
//////////////////////////////////主页APP JSON处理
#include <cJSON.h>

static char jsonbuffer[1024];
const char default_weather_key[] = "SoC098cCa8Ih-GWTb";
const char default_weather_city[] = "北京";
const char default_weather_provider[] = "openmeteo";
const char default_weather_endpoint[] = "http://api.open-meteo.com";
const char default_weather_lat[] = "39.9042";
const char default_weather_lon[] = "116.4074";
const char default_app_setting[] = "{\"weather\":\"\",\"city\":\"北京\",\"weather_provider\":\"openmeteo\",\"weather_endpoint\":\"http://api.open-meteo.com\",\"weather_lat\":\"39.9042\",\"weather_lon\":\"116.4074\"}";

struct app_setting app_settings_save;

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
           (strcmp(provider, "openmeteo") == 0 ||
            strcmp(provider, "seniverse") == 0 ||
            strcmp(provider, "qweather") == 0);
}

static const char *defaultWeatherEndpointForProvider(const char *provider)
{
    if (provider != NULL && strcmp(provider, "seniverse") == 0)
        return "http://api.seniverse.com";
    if (provider != NULL && strcmp(provider, "qweather") == 0)
        return "https://devapi.qweather.com";
    return default_weather_endpoint;
}

static void fillWeatherDefaultsIfMissing()
{
    if (!isKnownWeatherProvider(app_settings_save.weather_provider))
        copySettingString(app_settings_save.weather_provider, sizeof(app_settings_save.weather_provider), default_weather_provider);

    if (strlen(app_settings_save.weather_endpoint) == 0)
        copySettingString(app_settings_save.weather_endpoint, sizeof(app_settings_save.weather_endpoint), defaultWeatherEndpointForProvider(app_settings_save.weather_provider));

    if (strcmp(app_settings_save.weather_provider, "openmeteo") == 0)
    {
        app_settings_save.weather_secret[0] = '\0';
        if (strlen(app_settings_save.weather_city) == 0)
            copySettingString(app_settings_save.weather_city, sizeof(app_settings_save.weather_city), default_weather_city);
        if (strcmp(app_settings_save.weather_city, default_weather_city) == 0 && strlen(app_settings_save.weather_lat) == 0)
            copySettingString(app_settings_save.weather_lat, sizeof(app_settings_save.weather_lat), default_weather_lat);
        if (strcmp(app_settings_save.weather_city, default_weather_city) == 0 && strlen(app_settings_save.weather_lon) == 0)
            copySettingString(app_settings_save.weather_lon, sizeof(app_settings_save.weather_lon), default_weather_lon);
    }
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
    }
    else
    {
        app_settings_save.weather_secret[0] = '\0';
        copySettingString(app_settings_save.weather_provider, sizeof(app_settings_save.weather_provider), default_weather_provider);
        copySettingString(app_settings_save.weather_endpoint, sizeof(app_settings_save.weather_endpoint), default_weather_endpoint);
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
    copySettingString(app_settings_save.weather_lat, sizeof(app_settings_save.weather_lat), legacy.weather_lat);
    copySettingString(app_settings_save.weather_lon, sizeof(app_settings_save.weather_lon), legacy.weather_lon);
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

    const char *weather = jsonStringValue(root, "weather");
    if (weather != NULL && strlen(weather) > 0)
        copySettingString(app_settings_save.weather_secret, sizeof(app_settings_save.weather_secret), weather);

    const char *city = jsonStringValue(root, "city");
    if (city != NULL && strlen(city) > 0)
        copySettingString(app_settings_save.weather_city, sizeof(app_settings_save.weather_city), city);

    const char *weather_provider = jsonStringValue(root, "weather_provider");
    if (weather_provider != NULL && strlen(weather_provider) > 0)
        copySettingString(app_settings_save.weather_provider, sizeof(app_settings_save.weather_provider), weather_provider);

    const char *weather_endpoint = jsonStringValue(root, "weather_endpoint");
    if (weather_endpoint != NULL && strlen(weather_endpoint) > 0)
        copySettingString(app_settings_save.weather_endpoint, sizeof(app_settings_save.weather_endpoint), weather_endpoint);

    const char *weather_lat = jsonStringValue(root, "weather_lat");
    if (weather_lat != NULL)
        copySettingString(app_settings_save.weather_lat, sizeof(app_settings_save.weather_lat), weather_lat);

    const char *weather_lon = jsonStringValue(root, "weather_lon");
    if (weather_lon != NULL)
        copySettingString(app_settings_save.weather_lon, sizeof(app_settings_save.weather_lon), weather_lon);

    fillWeatherDefaultsIfMissing();
    cJSON_Delete(root);
}

void appSettingsToJson(char *result)
{
    cJSON *item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "weather_provider", app_settings_save.weather_provider);
    cJSON_AddStringToObject(item, "weather_endpoint", app_settings_save.weather_endpoint);
    cJSON_AddStringToObject(item, "weather_lat", app_settings_save.weather_lat);
    cJSON_AddStringToObject(item, "weather_lon", app_settings_save.weather_lon);
    cJSON_AddBoolToObject(item, "weather_configured", strlen(app_settings_save.weather_secret) > 0 || strcmp(app_settings_save.weather_provider, "openmeteo") == 0);
    cJSON_AddStringToObject(item, "city", app_settings_save.weather_city);
    writeJsonToBuffer(item, result, 1024);
    cJSON_Delete(item);
}

void HAL::saveAppSettings()
{
    File file = LittleFS.open("/.cfg.bin", "w");
    file.write((uint8_t *)&app_settings_save, sizeof(app_settings_save));
    file.close();
}

void HAL::loadAppSettings()
{
    File file = LittleFS.open("/.cfg.bin", "r");
    if (file)
    {
        size_t fileSize = file.size();
        if (fileSize == sizeof(app_settings_save))
        {
            int sz = file.read((uint8_t *)&app_settings_save, sizeof(app_settings_save));
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
            fillWeatherDefaultsIfMissing();
            return;
        }
        if (fileSize == sizeof(extended_app_setting))
        {
            extended_app_setting legacy;
            int sz = file.read((uint8_t *)&legacy, sizeof(legacy));
            file.close();
            if (sz != sizeof(legacy))
                goto reset;
            migrateExtendedAppSettings(legacy);
            saveAppSettings();
            return;
        }
        if (fileSize == sizeof(legacy_app_setting))
        {
            legacy_app_setting legacy;
            int sz = file.read((uint8_t *)&legacy, sizeof(legacy));
            file.close();
            if (sz != sizeof(legacy))
                goto reset;
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
        parseAppSettings(default_app_setting);
        file = LittleFS.open("/.cfg.bin", "w");
        file.write((uint8_t *)&app_settings_save, sizeof(app_settings_save));
        file.close();
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
        sendGzippedAsset("application/javascript", __web_index_js_gz, sizeof(__web_index_js_gz));
    });
    server.on("/ffmpeg.js", HTTP_GET, []() {
        sendGzippedAsset("application/javascript", __web_ffmpeg_js_gz, sizeof(__web_ffmpeg_js_gz));
    });
    server.on("/index.css", HTTP_GET, []() {
        sendGzippedAsset("text/css", __web_index_css_gz, sizeof(__web_index_css_gz));
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
    }, "webserver", 4096, NULL, 3, &webserver_task, 1);

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
