#include "A_Config.h"
#include <WiFi.h>

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

void task_webserver_autostart(void *p)
{
    vTaskDelay(pdMS_TO_TICKS(8000));
    if (hal.server_started || WiFiMgr.count() == 0)
    {
        vTaskDelete(NULL);
        return;
    }

    if (WiFiMgr.autoConnectSaved(6000))
    {
        hal.start_webserver();
    }
    vTaskDelete(NULL);
}

static void start_webserver_early()
{
    if (hal.server_started || WiFiMgr.count() == 0)
        return;

    if (WiFiMgr.autoConnectSaved(6000))
    {
        hal.start_webserver();
    }
}

void debug_USB_UART(void *p)
{
    bool in_setting_mode = false;
    bool in_qrcode_mode = false;
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
        case 'q': // 进入二维码模式
            in_qrcode_mode = !in_qrcode_mode;
            if (in_qrcode_mode)
                hal.send_sysctl(EVENT_GOTO_QRCODE);
            else
                hal.send_sysctl(EVENT_EXIT_QRCODE);
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
            ESP_LOGW("WiFiManager", "HEAP: %"PRIu32, esp_get_free_internal_heap_size());
            ESP_LOGW("WiFiManager", "HEAP_SPI: %"PRIu32, esp_get_free_heap_size());
            break;
        case 'k':
            hal.send_sysctl(EVENT_KB_KEYPRESS);
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
AppJPG appJPG;
AppWeather appWeather;
AppSettings appSettings;
AppQRCode appQRCode;
extern void add_to_app_list(BaseApp *app);
uint32_t RTC_DATA_ATTR last_appid = 0;
#include "boot_animation.h"
#include <driver/rtc_io.h>
extern "C" void app_main()
{
    initArduino();
    psramInit();
    hal.init();
    hal.getTime();
    hal.kb_status.channel_current = 0;
    WiFiMgr.init();

    start_webserver_early();
    appHome.init();
    appAPS.init();
    appGIF.init();
    appJPG.init();
    appWeather.init();
    appSettings.init();
    appQRCode.init();
    appManagerLite.appHome = &appHome;
    appManagerLite.appAPS = &appAPS;
    appManagerLite.appGIF = &appGIF;
    appManagerLite.appJPG = &appJPG;
    appManagerLite.appWeather = &appWeather;
    appManagerLite.appSettings = &appSettings;
    appManagerLite.appQRCode = &appQRCode;

    if (last_appid == 0) {
        last_appid = hal.pref.getInt("last_appid", 1);
    }
    if (last_appid == 100 || last_appid == 6 || last_appid == 7 || last_appid == 8) {
        last_appid = 1;
    }
    // 开机动画
    
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED && esp_reset_reason() != ESP_RST_SW)
    {
        rtc_gpio_deinit((gpio_num_t)PIN_SERIAL1_RX);
        if (hal.config_bootanimation)
            videoPlayer.playBuffer(__boot_mpeg, sizeof(__boot_mpeg));
    }
    protocol_init();
    xTaskCreatePinnedToCore(task_lvgl_update, "lvgl_update", 1024 * 6, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(debug_USB_UART, "debug_USB_UART", 1024 * 4, NULL, 6, NULL, 1);
    xTaskCreatePinnedToCore(task_webserver_autostart, "web_autostart", 4096, NULL, 1, NULL, 0);
    if (WiFiMgr.count() > 0) {
        appManagerLite.init(last_appid);
    } else {
        appManagerLite.init(100);
    }

    vTaskDelete(NULL);
    vTaskDelay(portMAX_DELAY);
}
