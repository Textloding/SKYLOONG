#include "A_Config.h"
HAL hal;
esp_lcd_panel_handle_t panel_handle = NULL;
esp_lcd_panel_io_handle_t io_handle = NULL;
#define DRAW_BUF_SIZE (2 * screenWidth * screenHeight)
lv_indev_t *indev_keypad;
static int first_refresh = 3;
static inline void swapBuffer(uint16_t *buffer, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++)
    {
        buffer[i] = (buffer[i] >> 8) | (buffer[i] << 8);
    }
}
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    swapBuffer((uint16_t *)color_p, (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1));
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
    lv_label_set_text(lbl_title, _tr(I18N_ID_NEED_ACTION));
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_t *line1;
    static lv_point_t line_points[] = {{0, 0}, {150, 0}};
    line1 = lv_line_create(box_enter_settings);
    lv_line_set_points(line1, line_points, 2); /*Set the points*/
    lv_obj_align(line1, LV_ALIGN_TOP_MID, 0, 50);

    lv_obj_t *label = lv_label_create(box_enter_settings);
    lv_obj_set_style_text_font(label, &lv_font_chinese_16, 0);
    lv_label_set_text(label, _tr(I18N_ID_NEED_ACTION_DESC));
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
    if (hal.battery_pct != 0)
    {
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
            // if (event.data == 0)
            // {
            //     if (xSemaphoreTake(hal._mutex, 2000) == pdTRUE)
            //     {
            //         xSemaphoreGive(hal._mutex);
            //         ledcWrite(7, 0);
            //     }
            // }
            // else
            // {
            //     hal.setBrightness(hal._brightness);
            // }
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
    spi_bus_config_t buscfg;
    memset(&buscfg, 0, sizeof(spi_bus_config_t));
    buscfg.mosi_io_num = PIN_DISPLAY_MOSI;
    buscfg.miso_io_num = -1;
    buscfg.sclk_io_num = PIN_DISPLAY_SCLK;
    buscfg.quadwp_io_num = -1; // Quad SPI LCD driver is not yet supported
    buscfg.quadhd_io_num = -1; // Quad SPI LCD driver is not yet supported
    buscfg.max_transfer_sz = 320 * 240 * 2;
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO)); // Enable the DMA feature
    esp_lcd_panel_io_spi_config_t io_config;
    memset(&io_config, 0, sizeof(esp_lcd_panel_io_spi_config_t));
    io_config.cs_gpio_num = PIN_DISPLAY_CS;
    io_config.dc_gpio_num = PIN_DISPLAY_DC;
    io_config.spi_mode = 0;
    io_config.pclk_hz = 80000000;
    io_config.trans_queue_depth = 2;
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle));
    esp_lcd_panel_dev_config_t panel_config;
    memset(&panel_config, 0, sizeof(esp_lcd_panel_dev_config_t));
    panel_config.reset_gpio_num = PIN_DISPLAY_RST;
    panel_config.color_space = ESP_LCD_COLOR_SPACE_RGB;
    panel_config.bits_per_pixel = 16;
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
    WiFi.setHostname("GKScreen");
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
    loadAppSettings();
    pref.begin("settings", false);
    hal._brightness = pref.getUInt("bright", 6);
    hal.config_time_12hr = pref.getBool("12hr", false);
    hal.config_show_battery_value = pref.getBool("s_b_v", false);
    hal.config_statusbar_center = pref.getBool("s_c", false);
    hal.config_show_boot_animation = hal.pref.getBool("b_a", false);
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
        ESP_LOGE("FileCopy", "no memory");
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

#include <WebServer.h>
#include <ESPmDNS.h>

#define FILESYSTEM LittleFS
WebServer server(80);
// holds the current upload
File fsUploadFile;
#include <list>
// format bytes
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
            ESP_LOGE("SERVER", "[文件] 无法打开目录");
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
                    ESP_LOGE("SERVER", "无法写入文件");
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
String formatBytes(size_t bytes)
{
    if (bytes < 1024)
    {
        return String(bytes) + "B";
    }
    else if (bytes < (1024 * 1024))
    {
        return String(bytes / 1024.0) + "KB";
    }
    else if (bytes < (1024 * 1024 * 1024))
    {
        return String(bytes / 1024.0 / 1024.0) + "MB";
    }
    else
    {
        return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
    }
}

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
        ESP_LOGI("SERVER", "handleFileUpload Size? %d", upload.totalSize);
        fsUploadFile = FILESYSTEM.open(filename, "w");
        filename = String();
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        // ESP_LOGI("SERVER", "handleFileUpload Data: "); ESP_LOGI("SERVER", upload.currentSize);
        if (fsUploadFile)
        {
            fsUploadFile.write(upload.buf, upload.currentSize);
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (fsUploadFile)
        {
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
    FILESYSTEM.remove(path);
    server.send(200, "text/plain", "");
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

    String output = "[";
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
            if (output != "[")
            {
                output += ',';
            }
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
    output += "]";
    server.send(200, "text/json", output);
}

void handleRMRF()
{
    if (server.hasArg("path"))
    {
        String path = server.arg("path");
        if (path == "")
        {
            server.send(500, "text/plain", "ERR 500");
        }
        hal.rm_rf((String("/littlefs/") + path).c_str());
        server.send(200, "text/plain", "OK");
        return;
    }
    server.send(500, "text/plain", "ERR 500");
}
void handleRename()
{
    if (server.hasArg("path") && server.hasArg("new"))
    {
        String path = server.arg("path");
        String newpath = server.arg("new");
        if (LittleFS.rename(path, newpath))
        {
            server.send(200, "text/plain", "OK");
            return;
        }
    }
    server.send(500, "text/plain", "ERR 500");
}
void handleMkdir()
{
    if (server.hasArg("path"))
    {
        String path = server.arg("path");
        if (LittleFS.mkdir(path))
        {
            server.send(200, "text/plain", "OK");
            return;
        }
    }
    server.send(500, "text/plain", "ERR 500");
}
void handleRoot()
{
    server.send(200, "text/html", "Hello from ESP32!");
}
//////////////////////////////////主页APP JSON处理
#include <cJSON.h>

static char jsonbuffer[2048];
const char default_app_setting[] = "[{\"widget\":1,\"time12\":false,\"bg\":false},{\"widget\":1,\"data\":0,\"showlbl\":true,\"lbl\":\"标题\",\"showindicator\":true,\"ext_url\":\"ws://192.168.1.2:8080/ws\",\"ext_interval\":200,\"ext_interpolation\":20,\"ext_zoom\":0.69},{\"widget\":0,\"data\":2,\"showlbl\":true,\"lbl\":\"APS\",\"showindicator\":true,\"ext_url\":\"ws://192.168.1.2:8080/ws\",\"ext_interval\":500,\"ext_interpolation\":40,\"ext_zoom\":0.05},{\"ip\":\"192.168.1.2\",\"port\":51648}]";

char app_settings_remote_ip[16];
uint16_t app_settings_remote_port;
struct app_setting app_settings_save[3];

void parseAppSettings(const char *input)
{
    cJSON *root = cJSON_Parse(input);
    if (root == NULL)
    {
        ESP_LOGE("APP", "JSON解析失败");
        cJSON_Delete(root);
        root = cJSON_Parse(default_app_setting);
    }
    cJSON *item = cJSON_GetArrayItem(root, 0);
    app_settings_save[0].widget = cJSON_GetObjectItem(item, "widget")->valueint;
    hal.config_time_12hr = cJSON_GetObjectItem(item, "time12")->valueint;
    app_settings_save[0].bg = cJSON_GetObjectItem(item, "bg")->valueint;
    item = cJSON_GetArrayItem(root, 1);
    app_settings_save[1].widget = cJSON_GetObjectItem(item, "widget")->valueint;
    app_settings_save[1].data = cJSON_GetObjectItem(item, "data")->valueint;
    strncpy(app_settings_save[1].ext_url, cJSON_GetObjectItem(item, "ext_url")->valuestring, 128);
    app_settings_save[1].ext_interval = cJSON_GetObjectItem(item, "ext_interval")->valueint;
    app_settings_save[1].ext_interpolation = cJSON_GetObjectItem(item, "ext_interpolation")->valueint;
    app_settings_save[1].ext_zoom = cJSON_GetObjectItem(item, "ext_zoom")->valuedouble;
    app_settings_save[1].showlbl = cJSON_GetObjectItem(item, "showlbl")->valueint;
    strncpy(app_settings_save[1].lbl, cJSON_GetObjectItem(item, "lbl")->valuestring, 128);
    app_settings_save[1].showindicator = cJSON_GetObjectItem(item, "showindicator")->valueint;
    item = cJSON_GetArrayItem(root, 2);
    app_settings_save[2].widget = cJSON_GetObjectItem(item, "widget")->valueint;
    app_settings_save[2].data = cJSON_GetObjectItem(item, "data")->valueint;
    strncpy(app_settings_save[2].ext_url, cJSON_GetObjectItem(item, "ext_url")->valuestring, 128);
    app_settings_save[2].ext_interval = cJSON_GetObjectItem(item, "ext_interval")->valueint;
    app_settings_save[2].ext_interpolation = cJSON_GetObjectItem(item, "ext_interpolation")->valueint;
    app_settings_save[2].ext_zoom = cJSON_GetObjectItem(item, "ext_zoom")->valuedouble;
    app_settings_save[2].showlbl = cJSON_GetObjectItem(item, "showlbl")->valueint;
    strncpy(app_settings_save[2].lbl, cJSON_GetObjectItem(item, "lbl")->valuestring, 128);
    app_settings_save[2].showindicator = cJSON_GetObjectItem(item, "showindicator")->valueint;
    item = cJSON_GetArrayItem(root, 3);
    strncpy(app_settings_remote_ip, cJSON_GetObjectItem(item, "ip")->valuestring, 15);
    app_settings_remote_port = cJSON_GetObjectItem(item, "port")->valueint;
}

void appSettingsToJson(char *result)
{
    cJSON *root = cJSON_CreateArray();
    cJSON *item = cJSON_CreateObject();
    cJSON_AddNumberToObject(item, "widget", app_settings_save[0].widget);
    cJSON_AddBoolToObject(item, "time12", hal.config_time_12hr);
    cJSON_AddBoolToObject(item, "bg", app_settings_save[0].bg);
    cJSON_AddItemToArray(root, item);
    item = cJSON_CreateObject();
    cJSON_AddNumberToObject(item, "widget", app_settings_save[1].widget);
    cJSON_AddNumberToObject(item, "data", app_settings_save[1].data);
    cJSON_AddStringToObject(item, "ext_url", app_settings_save[1].ext_url);
    cJSON_AddNumberToObject(item, "ext_interval", app_settings_save[1].ext_interval);
    cJSON_AddNumberToObject(item, "ext_interpolation", app_settings_save[1].ext_interpolation);
    cJSON_AddNumberToObject(item, "ext_zoom", app_settings_save[1].ext_zoom);
    cJSON_AddBoolToObject(item, "showlbl", app_settings_save[1].showlbl);
    cJSON_AddStringToObject(item, "lbl", app_settings_save[1].lbl);
    cJSON_AddBoolToObject(item, "showindicator", app_settings_save[1].showindicator);
    cJSON_AddItemToArray(root, item);
    item = cJSON_CreateObject();
    cJSON_AddNumberToObject(item, "widget", app_settings_save[2].widget);
    cJSON_AddNumberToObject(item, "data", app_settings_save[2].data);
    cJSON_AddStringToObject(item, "ext_url", app_settings_save[2].ext_url);
    cJSON_AddNumberToObject(item, "ext_interval", app_settings_save[2].ext_interval);
    cJSON_AddNumberToObject(item, "ext_interpolation", app_settings_save[2].ext_interpolation);
    cJSON_AddNumberToObject(item, "ext_zoom", app_settings_save[2].ext_zoom);
    cJSON_AddBoolToObject(item, "showlbl", app_settings_save[2].showlbl);
    cJSON_AddStringToObject(item, "lbl", app_settings_save[2].lbl);
    cJSON_AddBoolToObject(item, "showindicator", app_settings_save[2].showindicator);
    cJSON_AddItemToArray(root, item);
    item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "ip", app_settings_remote_ip);
    cJSON_AddNumberToObject(item, "port", app_settings_remote_port);
    cJSON_AddItemToArray(root, item);
    strncpy(result, cJSON_PrintUnformatted(root), 2048);
    cJSON_Delete(root);
}

void HAL::saveAppSettings()
{
    File file = LittleFS.open("/.cfg.bin", "w");
    file.write((uint8_t *)&app_settings_save, sizeof(app_settings_save));
    file.write((uint8_t *)app_settings_remote_ip, 16);
    file.write((uint8_t *)&app_settings_remote_port, 2);
    hal.pref.putBool("12hr", hal.config_time_12hr);
    file.close();
}

void HAL::loadAppSettings()
{
    File file = LittleFS.open("/.cfg.bin", "r");
    if (file)
    {
        file.read((uint8_t *)&app_settings_save, sizeof(app_settings_save));
        file.read((uint8_t *)app_settings_remote_ip, 16);
        file.read((uint8_t *)&app_settings_remote_port, 2);
        file.close();
    }
    else
    {
        parseAppSettings(default_app_setting);
        file = LittleFS.open("/.cfg.bin", "w");
        file.write((uint8_t *)&app_settings_save, sizeof(app_settings_save));
        file.write((uint8_t *)app_settings_remote_ip, 16);
        file.write((uint8_t *)&app_settings_remote_port, 2);
        file.close();
    }
}
void handleJson()
{
    if (server.method() == HTTP_POST && server.hasArg("plain"))
    {
        parseAppSettings(server.arg("plain").c_str());
        hal.saveAppSettings();
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
#include "webserver/index.h"
#include "webserver/favicon.h"
#include "webserver/csss.h"
#include "webserver/jq.h"
#include "webserver/mdb.h"

void HAL::start_webserver()
{
    if (WiFi.getMode() == WIFI_OFF)
    {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("GKScreen", "12345678");
    }
    MDNS.begin("gkscreen");
    if (LittleFS.exists("/.data") == false)
    {
        LittleFS.mkdir("/.data");
    }
    server.on("/list", HTTP_GET, handleFileList);
    server.on("/edit", HTTP_PUT, handleFileCreate);    // create file
    server.on("/edit", HTTP_DELETE, handleFileDelete); // delete file
    // first callback is called after the request has ended with all parsed arguments
    // second callback handles file uploads at that location
    server.on(
        "/edit", HTTP_POST, []()
        { server.send(200, "text/plain", ""); },
        handleFileUpload);

    server.on("/rmrf", HTTP_POST, handleRMRF);
    server.on("/mkdir", HTTP_POST, handleMkdir);
    server.on("/rename", HTTP_POST, handleRename);
    server.on("/reboot", HTTP_POST, []()
              {
        server.send(200, "text/plain", "OK");
        delay(100);
        ESP.restart(); });
    server.on("/config.json", handleJson);
    server.on("/", HTTP_GET, []()
              { server.sendHeader("Content-Encoding", "gzip", true);server.send_P(200, "text/html", (const char *)__web_index_htm_gz, sizeof(__web_index_htm_gz)); });
    server.on("/favicon.ico", HTTP_GET, []()
              { server.sendHeader("Content-Encoding", "gzip", true);server.send_P(200, "image/x-icon", (const char *)__web_favicon_ico_gz, sizeof(__web_favicon_ico_gz)); });
    server.on("/css/csss.css", HTTP_GET, []()
              { server.sendHeader("Content-Encoding", "gzip", true);server.send_P(200, "text/css", (const char *)__web_css_csss_css_gz, sizeof(__web_css_csss_css_gz)); });
    server.on("/js/jq.js", HTTP_GET, []()
              { server.sendHeader("Content-Encoding", "gzip", true);server.send_P(200, "application/javascript", (const char *)__web_js_jq_js_gz, sizeof(__web_js_jq_js_gz)); });
    server.on("/js/mdb.js", HTTP_GET, []()
              { server.sendHeader("Content-Encoding", "gzip", true);server.send_P(200, "application/javascript", (const char *)__web_js_mdb_js_gz, sizeof(__web_js_mdb_js_gz)); });
    server.onNotFound([]()
                      {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "FileNotFound");
    } });

    server.begin();
    ESP_LOGI("SERVER", "HTTP server started");
    hal.server_started = true;
    GUI::toast(_tr(I18N_ID_SERVER_STARTED));
    xTaskCreatePinnedToCore([](void *p)
                            {
        while (1)
        {if(hal.server_started) {server.handleClient();}vTaskDelay(5);} },
                            "webserver", 4096, NULL, 3, &webserver_task, 1);
}

void HAL::stop_webserver()
{
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