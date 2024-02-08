#include "A_Config.h"
HAL hal;
esp_lcd_panel_handle_t panel_handle = NULL;
esp_lcd_panel_io_handle_t io_handle = NULL;
#define DRAW_BUF_SIZE (2 * screenWidth * screenHeight)
lv_indev_t *indev_keypad;
static int first_refresh = 2;
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, (uint16_t *)&color_p->full);
    if (first_refresh > 0)
    {
        first_refresh--;
        if (first_refresh == 0)
        {
            hal.setBrightness(hal._brightness);
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
static lv_obj_t *box_enter_settings = NULL;
static bool current_require_setting = false; // 当前是否要求进入设置模式
static void create_box_enter_settings()
{
    hal.LOCKLV();
    if (box_enter_settings != NULL)
        lv_obj_del(box_enter_settings);
    box_enter_settings = lv_obj_create(lv_layer_top());
    lv_obj_set_size(box_enter_settings, 300, 170);
    lv_obj_align(box_enter_settings, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *lbl_title = lv_label_create(box_enter_settings);
    lv_obj_set_style_text_font(lbl_title, &lv_font_big_32, 0);
    lv_label_set_text(lbl_title, "需要进行操作");
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_t *line1;
    static lv_point_t line_points[] = {{0, 0}, {150, 0}};
    line1 = lv_line_create(box_enter_settings);
    lv_line_set_points(line1, line_points, 2); /*Set the points*/
    lv_obj_align(line1, LV_ALIGN_TOP_MID, 0, 50);

    lv_obj_t *label = lv_label_create(box_enter_settings);
    lv_obj_set_style_text_font(label, &lv_font_chinese_16, 0);
    lv_label_set_text(label, "  此功能要求进入“设置模式”，但键盘侧未提供相关API，请通过组合键手动进入此模式。");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_set_width(label, 260);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_pop_up(box_enter_settings);
    hal.UNLOCKLV();
}
static void delete_box_enter_settings()
{
    if (box_enter_settings == NULL)
        return;
    hal.LOCKLV();
    lv_obj_fall_down(box_enter_settings);
    lv_obj_del_delayed(box_enter_settings, 310);
    hal.UNLOCKLV();
    box_enter_settings = NULL;
}
static lv_obj_t *status_bar_A = NULL;
static lv_obj_t *status_bar_S = NULL;
static lv_obj_t *status_bar_1 = NULL;
static lv_obj_t *status_bar_winLK = NULL;
static lv_obj_t *status_bar_charging = NULL;
static lv_obj_t *status_bar_battery = NULL;
static lv_obj_t *status_bar_system = NULL; // Windows或mac
static lv_obj_t *status_bar_Bluetooth = NULL;
static lv_obj_t *status_bar_24 = NULL;
static lv_obj_t *status_bar_USB = NULL;
static lv_obj_t *status_bar_webserver = NULL;

void update_kb_lock(lv_obj_t *&obj, bool lock, const char *symbol, lv_state_t state)
{
    if (lock)
    {
        if (obj == NULL)
            obj = GUI::status_bar_insert(symbol, 0);
        GUI::status_bar_status_set(obj, state);
    }
    else if (obj != NULL)
    {
        if (lv_obj_has_state(obj, state))
        {
            GUI::status_bar_status_set(obj, 0);
            obj->user_data = (void *)millis();
        }
        else if (millis() - (uint32_t)obj->user_data > 5000)
        {
            GUI::status_bar_remove(obj);
            obj = NULL;
        }
    }
}
void update_kb_connectivity(lv_obj_t *&obj, bool available, int channel_state, const char *symbol)
{
    static const lv_state_t state[4] = {0, LV_STATE_USER_1, LV_STATE_USER_1, LV_STATE_USER_2};
    if (available && channel_state != 0)
    {
        if (obj == NULL)
            obj = GUI::status_bar_insert(symbol, 0);
        if (state[channel_state] == LV_STATE_USER_1)
        {
            if (lv_obj_has_state(obj, LV_STATE_USER_1 | LV_STATE_USER_4) == false)
                GUI::status_bar_status_set(obj, state[channel_state]);
        }
        else if (lv_obj_has_state(obj, state[channel_state]) == false)
            GUI::status_bar_status_set(obj, state[channel_state]);
        obj->user_data = 0;
    }
    else if (obj != NULL)
    {
        if (obj->user_data == 0)
        {
            GUI::status_bar_status_set(obj, 0);
            obj->user_data = (void *)millis();
        }
        else if (millis() - (uint32_t)obj->user_data > 5000)
        {
            GUI::status_bar_remove(obj);
            obj = NULL;
        }
    }
}

void update_status_bar()
{
    static uint8_t last_charging_state = 255;
    static uint8_t os = 2;
    int delay_total = -60;
    // 判断键盘锁定
    hal.LOCKLV();
    update_kb_lock(status_bar_winLK, hal.kb_status.winlk, SYMBOL_COMMAND_LOCK, LV_STATE_USER_3);
    update_kb_lock(status_bar_1, hal.kb_status.numlock, "1", LV_STATE_USER_2);
    update_kb_lock(status_bar_S, hal.kb_status.scrolllock, "S", LV_STATE_USER_2);
    update_kb_lock(status_bar_A, hal.kb_status.capslock, "A", LV_STATE_USER_2);
    update_kb_lock(status_bar_webserver, hal.server_started, SYMBOL_GLOBAL, LV_STATE_USER_2);
    // 判断通道
    update_kb_connectivity(status_bar_USB, hal.kb_status.channel_current == 3, hal.kb_status.chan_state, SYMBOL_USB);
    update_kb_connectivity(status_bar_Bluetooth, hal.kb_status.channel_current == 2 || hal.kb_status.channel_current == 1, hal.kb_status.chan_state, SYMBOL_BLUETOOTH);
    update_kb_connectivity(status_bar_24, hal.kb_status.channel_current == 0, hal.kb_status.chan_state, SYMBOL_24G);
    // 判断系统类型
    if (os != hal.kb_status.system)
    {
        os = hal.kb_status.system;
        if (os == 1)
            GUI::status_bar_replace(SYMBOL_WINDOWS, status_bar_system);
        else
            GUI::status_bar_replace(SYMBOL_APPLE, status_bar_system);
    }
    // 判断WINLK
    // 判断充电状态
    if (hal.battery_status != last_charging_state)
    {
        last_charging_state = hal.battery_status;
        if (hal.battery_status == BATTERY_STATUS_CHARGING)
        {
            if (status_bar_charging == NULL)
                status_bar_charging = GUI::status_bar_add(SYMBOL_CHARGING, (delay_total += 60));
            GUI::status_bar_status_set(status_bar_charging, LV_STATE_USER_1);
        }
        else if (hal.battery_status == BATTERY_STATUS_CHARGED)
        {
            if (status_bar_charging == NULL)
                status_bar_charging = GUI::status_bar_add(SYMBOL_CHARGING, (delay_total += 60));
            GUI::status_bar_status_set(status_bar_charging, LV_STATE_USER_2);
        }
        else if (status_bar_charging != NULL)
        {
            GUI::status_bar_status_set(status_bar_charging, 0);
            status_bar_charging->user_data = (void *)millis();
        }
    }
    if (status_bar_charging != NULL && (last_charging_state == 0 || last_charging_state == 3))
    {
        if (millis() - (uint32_t)status_bar_charging->user_data > 2000)
        {
            GUI::status_bar_remove(status_bar_charging);
            status_bar_charging = NULL;
        }
    }
    if (hal.battery_pct != 0)
    {
        if (status_bar_battery == NULL)
            status_bar_battery = GUI::status_bar_add(SYMBOL_BATTERY_100, (delay_total += 60));
        if (hal.config_show_battery_value)
        {
            lv_obj_set_style_text_font(status_bar_battery, &lv_font_chinese_16, 0);
            char buf[8];
            sprintf(buf, "%d", hal.battery_pct);
            lv_label_set_text(lv_obj_get_child(status_bar_battery, 0), buf);
        }
        else
        {
            lv_obj_set_style_text_font(status_bar_battery, &symbol_16, 0);
            if (hal.battery_pct < 20)
                lv_label_set_text(lv_obj_get_child(status_bar_battery, 0), SYMBOL_BATTERY_0);
            else if (hal.battery_pct < 40)
                lv_label_set_text(lv_obj_get_child(status_bar_battery, 0), SYMBOL_BATTERY_25);
            else if (hal.battery_pct < 60)
                lv_label_set_text(lv_obj_get_child(status_bar_battery, 0), SYMBOL_BATTERY_50);
            else if (hal.battery_pct < 80)
                lv_label_set_text(lv_obj_get_child(status_bar_battery, 0), SYMBOL_BATTERY_75);
            else
                lv_label_set_text(lv_obj_get_child(status_bar_battery, 0), SYMBOL_BATTERY_100);
        }
    }
    hal.UNLOCKLV();
}
static void task_systemctl(void *p)
{
    static bool sleep_status = false;
    while (1)
    {
        if (sleep_status == true)
        {
            hal.setBrightness(hal._brightness);
            sleep_status = false;
        }
        sysctl_event_t event;
        xQueueReceive(hal._queue, &event, portMAX_DELAY);
        switch (event.type)
        {
        case EVENT_GET_TIME:
            DS1302_getDateTime(&hal.rtc, &hal.datetime);
            break;
        case EVENT_GOTO_SETTING:
            hal.setting_mode = true;
            if (current_require_setting == true)
                delete_box_enter_settings();
            break;
        case EVENT_EXIT_SETTING:
            hal.setting_mode = false;
            if (current_require_setting == true)
                create_box_enter_settings();
            break;
        case EVENT_REQUIRE_SETTING:
        {
            current_require_setting = event.data;
            if (current_require_setting == true)
            {
                if (hal.setting_mode == false)
                    create_box_enter_settings();
            }
            else
            {
                delete_box_enter_settings();
            }
            break;
        }
        case EVENT_TOGGLE_SCREEN_ON:
            if (event.data == 0)
            {
                if (xSemaphoreTake(hal._mutex, 2000) == pdTRUE)
                {
                    xSemaphoreGive(hal._mutex);
                    GUI::toast("键盘叫我睡，但他不停给我发消息，我睡不着，只好先把屏幕背光关掉了");
                    delay(1000);
                    ledcWrite(7, 0);
                }
            }
            else
            {
                hal.setBrightness(hal._brightness);
            }
            break;
        case EVENT_APM_CHANGED:
            hal.APM = event.data;
            break;
        case EVENT_KB_STATUS_CHANGED:
            if (xSemaphoreTake(hal._mutex, 2000) == pdTRUE)
            {
                xSemaphoreGive(hal._mutex);
                update_status_bar();
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
            extern volatile bool _vid_stop;
            _vid_stop = true;
            if (xSemaphoreTake(hal._mutex, 2000) == pdTRUE)
            {
                if (hal.server_started == false)
                {
                    hal.goSleep();
                }
                else
                {
                    xSemaphoreGive(hal._mutex);
                    GUI::toast("键盘叫我睡，但我开着热点，不能睡，只好先把屏幕背光关掉了");
                    delay(1000);
                    ledcWrite(7, 0);
                    sleep_status = true;
                }
            }
            else
            {
                ESP_LOGE("HAL", "无法获取锁");
                hal.goSleep();
            }
        }
        break;
        default:
            break;
        }
    }
}
void demo()
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_DISPLAY_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = PIN_DISPLAY_SCLK,
        .quadwp_io_num = -1, // Quad SPI LCD driver is not yet supported
        .quadhd_io_num = -1, // Quad SPI LCD driver is not yet supported
        .max_transfer_sz = 320 * 240 * 2,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO)); // Enable the DMA feature
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = PIN_DISPLAY_CS,
        .dc_gpio_num = PIN_DISPLAY_DC,
        .spi_mode = 0,
        .pclk_hz = 80000000,
        .trans_queue_depth = 2,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle));
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_DISPLAY_RST,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
    };
    // Create LCD panel handle for ST7789, with the SPI IO device handle
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, true));
}

void HAL::init()
{
    pinMode(PIN_DISPLAY_PWR, OUTPUT);
    digitalWrite(PIN_DISPLAY_PWR, HIGH);
    ledcSetup(7, 16000, 8);
    ledcAttachPin(PIN_DISPLAY_BL, 7);
    ledcWrite(7, 0);
    demo();
    DS1302_begin(&rtc, PIN_RTC_SCLK, PIN_RTC_SDIO, PIN_RTC_RST);
    DS1302_writeClockRegister(&rtc, DS1302_REG_TC, 0xA6);
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
    pref.begin("settings", false);
    hal._brightness = pref.getUInt("bright", 6);
    hal.config_time_12hr = pref.getBool("12hr", false);
    hal.config_show_battery_value = pref.getBool("s_b_v", false);
    hal.config_statusbar_center = pref.getBool("s_c", false);
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
    /*Initialize the display*/
    lv_disp_drv_init(&disp_drv);
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    /*Register a keypad input device*/
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
    return 0; // return 0 if unable to get the time
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
        ledcWrite(7, brightness_lut[brightness]);
    }
    else
    {
        ESP_LOGE("HAL", "亮度设置错误: %d", brightness);
        ledcWrite(7, 130);
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

void HAL::goSleep()
{
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_SERIAL2_RX, 0);
    ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, LCD_CMD_SLPIN, NULL, 0));
    delay(2);
    esp_deep_sleep_start();
}

void HAL::requireSettings(bool req)
{
    send_sysctl(EVENT_REQUIRE_SETTING, req);
}
bool HAL::getLastRequireSettings()
{
    return current_require_setting;
}
void HAL::send_sysctl(system_event_type_t type, uint8_t data)
{
    sysctl_event_t event;
    event.type = type;
    event.data = data;
    xQueueSend(_queue, &event, portMAX_DELAY);
}
void HAL::copy(File &newFile, File &file)
{
    char *buf = (char *)malloc(512);
    size_t currentsize = 0;
    if (!buf)
    {
        Serial.println("内存已满");
        ESP.restart();
    }
    while (1)
    {
        currentsize = file.readBytes(buf, 512);
        if (currentsize == 0)
            break;
        newFile.write((uint8_t *)buf, currentsize);
    }
    free(buf);
}
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

void HAL::rm_rf(const char *path)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;

    // 打开目录
    if ((dp = opendir(path)) == NULL)
    {
        perror("opendir");
        return;
    }

    // 迭代读取目录中的文件
    while ((entry = readdir(dp)) != NULL)
    {
        // 获取文件的完整路径
        char filePath[257];
        snprintf(filePath, sizeof(filePath), "%s/%s", path, entry->d_name);

        // 获取文件信息
        if (stat(filePath, &statbuf) == -1)
        {
            perror("lstat");
            continue;
        }

        // 判断是否是目录
        if (S_ISDIR(statbuf.st_mode))
        {
            // 忽略.和..目录
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }

            // 递归删除子目录
            rm_rf(filePath);
        }
        else
        {
            // 删除文件
            if (remove(filePath) != 0)
            {
                perror("remove");
            }
        }
    }

    // 关闭目录
    closedir(dp);

    // 删除空目录
    if (rmdir(path) != 0)
    {
        perror("rmdir");
    }
}
//////////////////////////////////网页服务器部分
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <SPIFFSEditor.h>
#include <list>
AsyncWebServer server(80);
void rmrfHandler(AsyncWebServerRequest *request)
{
    if (request->hasArg("path"))
    {
        String path = request->arg("path");
        if (path == "")
        {
            request->send(500, "text/plain", "EER");
        }
        hal.rm_rf((String("/littlefs/") + path).c_str());
        request->send(200, "text/plain", "OK");
        return;
    }
    request->send(500, "text/plain", "EER");
}
void renameHandler(AsyncWebServerRequest *request)
{
    if (request->hasArg("path") && request->hasArg("new"))
    {
        String path = request->arg("path");
        String newpath = request->arg("new");
        if (LittleFS.rename(path, newpath))
        {
            request->send(200, "text/plain", "OK");
            return;
        }
    }
    request->send(500, "text/plain", "EER");
}
void mkdirHandler(AsyncWebServerRequest *request)
{
    if (request->hasArg("path"))
    {
        String path = request->arg("path");
        if (LittleFS.mkdir(path))
        {
            request->send(200, "text/plain", "OK");
            return;
        }
    }
    request->send(500, "text/plain", "EER");
}
bool myxcopy(const String path, const String newpath)
{
    std::list<String> filenames;
    File root, file;
    filenames.push_back(path);
    String tmp;
    while (filenames.empty() == false)
    {
        root = LittleFS.open(filenames.back());
        tmp = filenames.back();
        tmp.replace(path, newpath);
        filenames.pop_back();
        if (!root)
        {
            Serial.println("[文件] 无法打开目录");
            continue;
        }
        LittleFS.mkdir(tmp);
        file = root.openNextFile();
        while (file)
        {
            String name = file.name();
            if (file.isDirectory())
            {
                tmp = file.path();
                tmp.replace(path, newpath);
                LittleFS.mkdir(tmp);
                filenames.push_back(file.path());
            }
            else
            {
                // 复制文件
                tmp = file.path();
                tmp.replace(path, newpath);
                File newFile = LittleFS.open(tmp, "w");
                if (!newFile)
                {
                    // 打开失败
                    Serial.println("无法写入文件");
                    file.close();
                    root.close();
                    return false;
                }
                hal.copy(newFile, file);
                newFile.close();
                file.close();
            }
            file.close();
            file = root.openNextFile();
        }
    }
    root.close();
    return true;
}
void createAppHandler(AsyncWebServerRequest *request)
{
    if (request->hasArg("name"))
    {
        String name = request->arg("name");
        if (name == "")
        {
            request->send(500, "text/plain", "EER");
        }
        if (name.endsWith(".app") == false)
            name += ".app";
        hal.rm_rf((String("/littlefs/") + name).c_str());
        String currentPath = "/" + name;
        if (myxcopy("/webtmp", currentPath))
        {
            request->send(200, "text/plain", "OK");
            return;
        }
    }
    request->send(500, "text/plain", "EER");
}
static void sendreq(AsyncWebServerRequest *request, const char *mime, const uint8_t *name, unsigned int len)
{
    const char *buildTime = __DATE__ " " __TIME__ " GMT";
    if (request->header("If-Modified-Since").equals(buildTime))
    {
        request->send(304);
    }
    else
    {
        AsyncWebServerResponse *response = request->beginResponse_P(200, mime, name, len);
        response->addHeader("Content-Encoding", "gzip");
        response->addHeader("Last-Modified", buildTime);
        request->send(response);
    }
}
void HAL::start_webserver()
{
    WiFi.disconnect(true);
    WiFi.softAP("GKScreen");
    server.onNotFound([](AsyncWebServerRequest *request)
                      {
        if(WiFi.softAPgetStationNum() != 0)
        {
            request->redirect("http://192.168.4.1");
        }
        else
        {
            request->send(404);
        } });
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(404, "text/plain", "Not found"); }); //  sendreq(request, "text/html", __web_index_html_gz, __web_index_html_gz_len);
    server.addHandler(new SPIFFSEditor(LittleFS));
    server.on("/rmrf", HTTP_POST, rmrfHandler);
    server.on("/mkdir", HTTP_POST, mkdirHandler);
    server.on("/rename", HTTP_POST, renameHandler);
    server.begin();
    Serial.println("HTTP server started");
    hal.server_started = true;
    GUI::toast("服务器已启动");
}

void HAL::stop_webserver()
{
    server.end();
    WiFi.mode(WIFI_OFF);
    hal.server_started = false;
    GUI::toast("服务器已关闭");
}
