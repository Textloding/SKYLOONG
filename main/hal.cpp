#include "A_Config.h"

#include "Wire.h"
#include "es8311.h"
#include "ESP_I2S.h"
#include "keytone/keytone1.h"
#include "keytone/keytone2.h"
#include "keytone/keytone3.h"

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
    hal.aps_enable = pref.getBool("aps_enable", true);
    hal.weather_enable = pref.getBool("weather_enable", true);
    hal.sysinfo_enable = pref.getBool("sysinfo_enable", true);
    hal.gif_enable = pref.getBool("gif_enable", true);
    hal.jpg_enable = pref.getBool("jpg_enable", true);
    hal.config_time_roll = pref.getInt("t_r", 5000);
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

    xTaskCreatePinnedToCore(task_systemctl, "task_systemctl", 4096, NULL, 3, NULL, 0);
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

#define FILESYSTEM LittleFS
DNSServer dnsServer;
WebServer server(80);
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
const char default_app_setting[] = "{\"ip\":\"192.168.1.1\",\"port\":1648,\"weather\":\"SoC098cCa8Ih-GWTb\",\"city\":\"北京\",\"userdata\":\"请在网页端\n自定义文本\"}";

struct app_setting app_settings_save;

void parseAppSettings(const char *input)
{
    cJSON *root = cJSON_Parse(input);
    if (root == NULL)
    {
        ESP_LOGE("APP", "JSON解析失败");
        cJSON_Delete(root);
        root = cJSON_Parse(default_app_setting);
    }
    strncpy(app_settings_save.remote_ip, cJSON_GetObjectItem(root, "ip")->valuestring, sizeof(app_settings_save.remote_ip) - 1);
    app_settings_save.remote_port = cJSON_GetObjectItem(root, "port")->valueint;
    strncpy(app_settings_save.weather_secret, cJSON_GetObjectItem(root, "weather")->valuestring, sizeof(app_settings_save.weather_secret) - 1);
    strncpy(app_settings_save.weather_city, cJSON_GetObjectItem(root, "city")->valuestring, sizeof(app_settings_save.weather_city) - 1);
    strncpy(app_settings_save.userdata, cJSON_GetObjectItem(root, "userdata")->valuestring, sizeof(app_settings_save.userdata) - 1);
    cJSON_Delete(root);
}

void appSettingsToJson(char *result)
{
    cJSON *item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "ip", app_settings_save.remote_ip);
    cJSON_AddNumberToObject(item, "port", app_settings_save.remote_port);
    cJSON_AddStringToObject(item, "weather", app_settings_save.weather_secret);
    cJSON_AddStringToObject(item, "city", app_settings_save.weather_city);
    cJSON_AddStringToObject(item, "userdata", app_settings_save.userdata);
    strncpy(result, cJSON_PrintUnformatted(item), 1024);
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
        parseAppSettings(server.arg("plain").c_str());
        hal.saveAppSettings();
        hal.weather_update = false;
        hal.sysinfo_update = false;  
        server.send(200, "text/plain", "OK");
    }
    else if (server.method() == HTTP_GET)
    {
        appSettingsToJson(jsonbuffer);
        server.send(200, "application/json", jsonbuffer);
    }
    {
        server.send(500, "text/plain", "ERR 500");
    }
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
#include "webserver/cpu.h"
#include "webserver/demo.h"
#include "webserver/error_bg.h"
#include "webserver/error_m.h"
#include "webserver/ic_d.h"
#include "webserver/ic_del.h"
#include "webserver/image2.h"
#include "webserver/nothing.h"
#include "webserver/setting2.h"
#include "webserver/spead.h"
#include "webserver/theme1.h"
#include "webserver/theme2.h"
#include "webserver/theme3.h"
#include "webserver/time_bg.h"
#include "webserver/time_ic.h"
#include "webserver/video2.h"
#include "webserver/weather.h"
#include "webserver/worker.h"

void HAL::start_webserver()
{
    if (webserver_task != NULL)
        return;
    hal.server_started = true;

    if (WiFi.getMode() == WIFI_OFF || (WiFi.getMode() == WIFI_STA && WiFi.isConnected() == false))
    {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("SKYLOONG 4.0 Screen");
    }

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
        cJSON_AddBoolToObject(json, "sysinfo_enable", hal.sysinfo_enable);
        cJSON_AddBoolToObject(json, "gif_enable", hal.gif_enable);
        cJSON_AddBoolToObject(json, "jpg_enable", hal.jpg_enable);
        cJSON_AddNumberToObject(json, "time_roll", hal.config_time_roll);
        cJSON_AddStringToObject(json, "jpg_mode", hal.config_jpg_mode);
        cJSON_AddStringToObject(json, "jpg_file", hal.config_jpg_file);
        long timezone = i18n::getNTPOffset();
        timezone /= 3600;
        cJSON_AddNumberToObject(json, "timezone", timezone);
        cJSON_AddNumberToObject(json, "language", i18n::getLanguage());
        
        strncpy(jsonbuffer, cJSON_PrintUnformatted(json), 1024);
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

        strncpy(jsonbuffer, cJSON_PrintUnformatted(json), 1024);
        cJSON_Delete(json);
        server.send(200, "application/json", jsonbuffer);
    });

    server.on("/config_wifi", HTTP_POST, []() {
        if (server.hasArg("ssid") && server.hasArg("password")) {
            strcpy(hal.ssid, server.arg("ssid").c_str());
            strcpy(hal.password, server.arg("password").c_str());
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

    server.on("/config_app_sysinfo", HTTP_POST, []() {
        if (server.hasArg("enable")) {
            hal.sysinfo_enable = server.arg("enable").toBool();
            hal.pref.putBool("sysinfo_enable", hal.sysinfo_enable);
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
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "application/javascript", (const char *)__web_index_js_gz, sizeof(__web_index_js_gz));
    });
    server.on("/ffmpeg.js", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "application/javascript", (const char *)__web_ffmpeg_js_gz, sizeof(__web_ffmpeg_js_gz));
    });
    server.on("/index.css", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "text/css", (const char *)__web_index_css_gz, sizeof(__web_index_css_gz));
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
    server.on("/cpu.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_cpu_png_gz, sizeof(__web_cpu_png_gz));
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
    server.on("/nothing.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_nothing_png_gz, sizeof(__web_nothing_png_gz));
    });
    server.on("/setting2.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_setting2_png_gz, sizeof(__web_setting2_png_gz));
    });
    server.on("/spead.png", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "image/png", (const char *)__web_spead_png_gz, sizeof(__web_spead_png_gz));
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
    server.on("/assets/worker-43d19264.js", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip", true);
        server.send_P(200, "application/javascript", (const char *)__web_assets_worker_43d19264_js_gz, sizeof(__web_assets_worker_43d19264_js_gz));
    });

    server.begin();
    ESP_LOGI("SERVER", "HTTP server started");
    GUI::toast(_tr(I18N_ID_SERVER_STARTED));
    xTaskCreatePinnedToCore([](void *p)
    {
        while (1)
        {
            if(hal.server_started) {
                server.handleClient();
            }
            vTaskDelay(5);
        }
    }, "webserver", 4096, NULL, 3, &webserver_task, 1);
}

void HAL::stop_webserver()
{
    if (webserver_task == NULL)
        return;
    server.stop();
    if (WiFi.getMode() == WIFI_AP)
    {
        WiFi.mode(WIFI_OFF);
    }
    hal.server_started = false;
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
