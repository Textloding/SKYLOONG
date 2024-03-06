#include "A_Config.h"
#include <WiFi.h>

void drawDPPQRCode(const char *str);
void task_lvgl_update(void *p)
{
    while (1)
    {
        hal.LOCKLV();
        lv_timer_handler();
        hal.UNLOCKLV();
        delay(5);
    }
}
void debug_USB_UART(void *p)
{
    bool in_setting_mode = false;
    while (1)
    {
        int c = getchar();
        uint8_t send_data = 0;
        if (c < 0)
        {
            vTaskDelay(100);
            continue;
        }
        switch (c)
        {
        case '/': // 进入编辑模式（右下角两个同时长按）
            in_setting_mode = !in_setting_mode;
            if (in_setting_mode)
                hal.send_sysctl(EVENT_GOTO_SETTING);
            else
                hal.send_sysctl(EVENT_EXIT_SETTING);
            break;
        case '\b': // BackSpace
            // xSemaphoreGive(appManagerLite._binary_switchApp);
            break;
        case 'a': // 方向键向左
                send_data = LV_KEY_LEFT;
            break;
        case 'd': // 方向键向右
                send_data = LV_KEY_RIGHT;
            break;
        case 'w': // 方向键向上
            if (hal.lv_has_kb == true)
                send_data = LV_KEY_UP;
            else
                send_data = LV_KEY_PREV;
            break;
        case 's': // 方向键向下
            if (hal.lv_has_kb == true)
                send_data = LV_KEY_DOWN;
            else
                send_data = LV_KEY_NEXT;
            break;
        case '\n': // Enter
            send_data = LV_KEY_ENTER;
            break;
        case '`': // 菜单间切换（右下角+左上角短按）
            ESP_LOGW("debug_USB_UART", "Switch App");
            xSemaphoreGive(appManagerLite._binary_switchApp);
            break;
        case 'o':
            ESP_LOGW("WiFiManager", "HEAP: %d", esp_get_free_internal_heap_size());
            ESP_LOGW("WiFiManager", "HEAP_SPI: %d", esp_get_free_heap_size());
            break;
        default:
            break;
        }
        if (send_data)
        {
            xQueueSend(hal._queue_kb, &send_data, 0);
        }
    }
}
AppHome appHome;
AppAPS appAPS;
AppGIF appGIF;
AppWeather appWeather;
AppSysinfo appSysinfo;
AppSettings appSettings;
extern void add_to_app_list(BaseApp *app);
uint32_t RTC_DATA_ATTR last_appid = 0;
#include "boot_animation.h"
#include <driver/rtc_io.h>
extern "C" void app_main()
{
    initArduino();
    psramInit();
    hal.init();
    hal.kb_status.channel_current = 3;
    WiFiMgr.init();
    //////////////////////
    appHome.init();
    appAPS.init();
    appGIF.init();
    appWeather.init();
    appSysinfo.init();
    appSettings.init();
    add_to_app_list(&appHome);
    add_to_app_list(&appAPS);
    add_to_app_list(&appGIF);
    add_to_app_list(&appWeather);
    add_to_app_list(&appSysinfo);
    appManagerLite.appSettings = &appSettings;
    //////////////////////
    if (last_appid == 0)
    {
        last_appid = hal.pref.getInt("last_appid", 1);
    }
    // 开机动画
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED)
    {
        rtc_gpio_deinit((gpio_num_t)PIN_SERIAL2_RX);
        if (hal.config_bootanimation)
            videoPlayer.playBuffer(__boot_mpeg, sizeof(__boot_mpeg));
    }
    protocol_init();
    xTaskCreatePinnedToCore(task_lvgl_update, "lvgl_update", 1024 * 6, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(debug_USB_UART, "debug_USB_UART", 1024 * 4, NULL, 6, NULL, 1);
    if (DS1302_isHalted(&hal.rtc))
    {
        DS1302_halt(&hal.rtc, false);
        GUI::toast(_tr(I18N_ID_RTC_SHUTDOWN), true, 10000);
    }
    appManagerLite.init(last_appid);
    vTaskDelete(NULL);
    vTaskDelay(portMAX_DELAY);
}