#include <A_Config.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_dpp.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_task_wdt.h"
#include <driver/rtc_io.h>
#include <esp_netif.h>
#include <LittleFS.h>

#ifdef CONFIG_ESP_DPP_LISTEN_CHANNEL
#define EXAMPLE_DPP_LISTEN_CHANNEL_LIST CONFIG_ESP_DPP_LISTEN_CHANNEL_LIST
#else
#define EXAMPLE_DPP_LISTEN_CHANNEL_LIST "1"
#endif

#ifdef CONFIG_ESP_DPP_BOOTSTRAPPING_KEY
#define EXAMPLE_DPP_BOOTSTRAPPING_KEY CONFIG_ESP_DPP_BOOTSTRAPPING_KEY
#else
#define EXAMPLE_DPP_BOOTSTRAPPING_KEY 0
#endif

#ifdef CONFIG_ESP_DPP_DEVICE_INFO
#define EXAMPLE_DPP_DEVICE_INFO CONFIG_ESP_DPP_DEVICE_INFO
#else
#define EXAMPLE_DPP_DEVICE_INFO 0
#endif

//static const char *TAG = "wifi dpp-enrollee";
wifi_config_t s_dpp_wifi_config;

////static int s_retry_num = 0;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_dpp_event_group;

#define DPP_CONNECTED_BIT BIT0
#define DPP_CONNECT_FAIL_BIT BIT1
#define DPP_AUTH_FAIL_BIT BIT2
#define DPP_AUTH_CANCEL_BIT BIT3

static SemaphoreHandle_t manual_config_sem = NULL;
static char *manual_config_ssid = NULL;
static char *manual_config_password = NULL;

void build_scr_connecting(const char *ssid, const char *passwd)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
    lv_obj_t *label = lv_label_create(scr);
    lv_obj_set_style_text_font(label, &lv_font_chinese_16, 0);
    lv_label_set_text_fmt(label, _tr(I18N_ID_CONNECTING_TO), ssid);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_t *spinner = lv_spinner_create(scr, 1000, 60);

    lv_obj_set_size(spinner, 100, 100);
    lv_obj_center(spinner);
    strcpy(manual_config_ssid, ssid);
    strcpy(manual_config_password, passwd);
    xSemaphoreGive(manual_config_sem);
}
void cancel_connect()
{
    manual_config_ssid = NULL;
    xSemaphoreGive(manual_config_sem);
}
#include <WiFi.h>
#include <vector>
struct my_wifi_info_t {
    String ssid;
    int rssi;
    wifi_auth_mode_t auth;
};
LV_IMG_DECLARE(wifi_1);
LV_IMG_DECLARE(wifi_2);
LV_IMG_DECLARE(wifi_3);
LV_IMG_DECLARE(wifi_4);
LV_IMG_DECLARE(wifilock);
static void WiFiScanDone(lv_obj_t *box, int n)
{
    struct my_wifi_info_t tmp;
    std::vector<struct my_wifi_info_t> wifi_list;
    for(int i = 0; i < n; ++i)
    {
        tmp.ssid = WiFi.SSID(i);
        tmp.rssi = WiFi.RSSI(i);
        tmp.auth = WiFi.encryptionType(i);
        wifi_list.push_back(tmp);
    }
    std::sort(wifi_list.begin(), wifi_list.end(), [](const struct my_wifi_info_t &a, const struct my_wifi_info_t &b)
    {
        return a.rssi > b.rssi;
    });
    for (int i = 0; i < n; ++i)
    {
        lv_obj_t *btn = lv_btn_create(box);
        lv_obj_t *img_rssi = lv_img_create(btn);
        lv_obj_set_size(btn, lv_pct(100), 50);
        if (wifi_list[i].rssi > -50)
            lv_img_set_src(img_rssi, &wifi_4);
        else if (wifi_list[i].rssi > -65)
            lv_img_set_src(img_rssi, &wifi_3);
        else if (wifi_list[i].rssi > -75)
            lv_img_set_src(img_rssi, &wifi_2);
        else
            lv_img_set_src(img_rssi, &wifi_1);
        lv_obj_align(img_rssi, LV_ALIGN_LEFT_MID, 2, 0);
        lv_obj_t *label = lv_label_create(btn);
        lv_obj_set_style_text_font(label, &lv_font_chinese_16, 0);
        lv_label_set_text(label, wifi_list[i].ssid.c_str());
        lv_obj_align_to(label, img_rssi, LV_ALIGN_OUT_RIGHT_MID, 3, 0);
        if(wifi_list[i].auth != WIFI_AUTH_OPEN)
        {
            lv_obj_t *img_lock = lv_img_create(btn);
            lv_img_set_src(img_lock, &wifilock);
            lv_obj_align_to(img_lock, label, LV_ALIGN_OUT_RIGHT_MID, 3, 0);
        }
        lv_obj_add_event_cb(
            btn, [](lv_event_t *e)
            {
                lv_obj_t *scr;
                lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
                lv_obj_t *label = lv_obj_get_child(btn, 1);
                scr = lv_obj_get_screen(btn);

                static char ssid[64];
                strcpy(ssid, lv_label_get_text(label));
                if (WiFiMgr.has(ssid))
                {
                    hal.lv_has_kb = false;
                    build_scr_connecting(ssid, WiFiMgr.getPassword(ssid).c_str());
                    return;
                }
                if(lv_obj_get_child_cnt(btn) == 2)
                {
                    hal.lv_has_kb = false;
                    build_scr_connecting(ssid, "");
                    return;
                }
                lv_obj_t *box_passwd = lv_obj_create(scr);
                lv_obj_set_size(box_passwd, 280, 100);
                lv_obj_align(box_passwd, LV_ALIGN_TOP_MID, 0, 10);

                lv_obj_t *lbl_info = lv_label_create(box_passwd);
                lv_obj_set_style_text_font(lbl_info, &lv_font_chinese_16, 0);
                lv_label_set_text_fmt(lbl_info, _tr(I18N_ID_CONNECTED_TO), ssid);
                lv_obj_align(lbl_info, LV_ALIGN_TOP_MID, 0, 0);

                lv_obj_t *ta_passwd = lv_textarea_create(box_passwd);
                lv_obj_set_size(ta_passwd, 200, LV_SIZE_CONTENT);
                lv_obj_set_style_text_font(ta_passwd, &lv_font_chinese_16, 0);
                lv_textarea_set_placeholder_text(ta_passwd, _tr(I18N_ID_ENTER_PASSWORD));
                lv_obj_align(ta_passwd, LV_ALIGN_BOTTOM_MID, 0, 0);

                lv_obj_t *kb = lv_keyboard_create(scr);
                lv_keyboard_set_textarea(kb, ta_passwd);
                lv_obj_set_size(kb, lv_pct(100), lv_pct(50));
                lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
                lv_group_focus_obj(kb);
                hal.lv_has_kb = true;
                lv_obj_add_event_cb(
                    kb, [](lv_event_t *e)
                    {
                if (e->code == LV_EVENT_READY)
                {
                    if(strlen(lv_textarea_get_text(lv_keyboard_get_textarea((lv_obj_t *)lv_event_get_target(e)))) < 8)
                    {
                        GUI::toast(_tr(I18N_ID_PASSWORD_INVALID), false);
                        return;
                    }
                    hal.lv_has_kb = false;
                    build_scr_connecting(ssid, lv_textarea_get_text(lv_keyboard_get_textarea((lv_obj_t *)lv_event_get_target(e))));
                }
                else if (e->code == LV_EVENT_CANCEL)
                {
                    hal.lv_has_kb = false;
                    cancel_connect();
                } },
                    LV_EVENT_ALL, NULL);
                lv_obj_pop_up(box_passwd);
                lv_obj_pop_up(kb, 30, 300, 300); },
            LV_EVENT_CLICKED, NULL);
            lv_obj_add_event_cb(btn, [](lv_event_t *e)
            {
                if (lv_indev_get_key(lv_indev_get_act()) == LV_KEY_ESC)
                {
                    hal.lv_has_kb = false;
                    cancel_connect();
                }
            }, LV_EVENT_KEY, NULL);
            lv_obj_add_event_cb(btn, [](lv_event_t *e){
                lv_obj_scroll_to_view(lv_event_get_target(e), LV_ANIM_OFF);
            }, LV_EVENT_FOCUSED, NULL);
    }
    wifi_list.clear();
    WiFi.scanDelete();
}
void load_scr_manual()
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, false);
    lv_obj_t *label = lv_label_create(scr);
    lv_obj_set_style_text_font(label, &lv_font_chinese_16, 0);
    lv_label_set_text(label, _tr(I18N_ID_SELECT_WIFI));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 5);

    lv_obj_t *box = lv_obj_create(scr);
    lv_obj_set_size(box, 300, 200);
    lv_obj_align(box, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_flex_flow(box, LV_FLEX_FLOW_COLUMN);
    lv_obj_t *lbl_scanning = lv_label_create(box);
    lv_obj_set_style_text_font(lbl_scanning, &lv_font_chinese_16, 0);
    lv_label_set_text(lbl_scanning, _tr(I18N_ID_SCANNING));
    lv_obj_align(lbl_scanning, LV_ALIGN_TOP_MID, 0, 0);
    lv_timer_create([](lv_timer_t *timer)
                    {
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        int16_t n = WiFi.scanNetworks();
        if(n == WIFI_SCAN_FAILED)
        {
            GUI::toast(_tr(I18N_ID_WIFI_SCAN_FAILED), false);
        }
        else if(n == 0)
        {
            lv_label_set_text((lv_obj_t *)timer->user_data, _tr(I18N_ID_NO_NETWORK));
            return;
        }
        else if(n > 0)
        {
            WiFiScanDone(lv_obj_get_parent((lv_obj_t *)timer->user_data), n);
            lv_obj_del((lv_obj_t *)timer->user_data);
        }
        lv_timer_del(timer); },
                    800, lbl_scanning);
}

// void drawDPPQRCode(const char *str)
// {
//     hal.LOCKLV();
//     lv_obj_t *scr = lv_obj_create(NULL);
//     lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, false);

//     lv_obj_t *box = lv_obj_create(scr);

//     lv_obj_set_size(box, 190, 190);
//     lv_obj_align(box, LV_ALIGN_LEFT_MID, 10, 10);
//     lv_obj_set_style_radius(box, 10, 0);
//     lv_obj_set_style_pad_all(box, 2, 0);
//     lv_obj_t *qr = lv_qrcode_create(box, 170, lv_color_black(), lv_color_white());
//     const char *data = str;
//     lv_qrcode_update(qr, data, strlen(data));
//     lv_obj_center(qr);
//     lv_obj_t *label = lv_label_create(scr);
//     lv_obj_set_style_text_font(label, &lv_font_chinese_16, 0);
//     lv_label_set_text(label, "WiFi Easy Connect");
//     lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 5);

//     lv_obj_t *lbl_desc = lv_label_create(scr);
//     lv_obj_set_style_text_font(lbl_desc, &lv_font_chinese_16, 0);
//     lv_label_set_text(lbl_desc, "  请使用安卓手机系统设置扫描此二维码快速配网，或");
//     lv_obj_set_width(lbl_desc, 110);
//     lv_label_set_long_mode(lbl_desc, LV_LABEL_LONG_WRAP);
//     lv_obj_align_to(lbl_desc, box, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);

//     lv_obj_t *btn_manual = lv_btn_create(scr);
//     lv_obj_set_width(btn_manual, 100);
//     lv_obj_set_height(btn_manual, 35);
//     lv_obj_align_to(btn_manual, lbl_desc, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
//     lv_obj_set_style_radius(btn_manual, 10, 0);
//     lv_obj_set_style_pad_all(btn_manual, 2, 0);
//     lv_obj_t *lbl_manual = lv_label_create(btn_manual);
//     lv_obj_set_style_text_font(lbl_manual, &lv_font_chinese_16, 0);
//     lv_label_set_text(lbl_manual, "手动配置");
//     lv_obj_center(lbl_manual);
//     lv_obj_add_event_cb(
//         btn_manual, [](lv_event_t *e)
//         {
//         if (e->code == LV_EVENT_CLICKED)
//         {
//             xSemaphoreTake(manual_config_sem, portMAX_DELAY);
//             xEventGroupSetBits(s_dpp_event_group, DPP_AUTH_CANCEL_BIT);         //通知DPP配网取消
//             load_scr_manual();
//         } },
//         LV_EVENT_CLICKED, NULL);

//     lv_obj_t *btn_cancel = lv_btn_create(scr);
//     lv_obj_set_width(btn_cancel, 100);
//     lv_obj_set_height(btn_cancel, 35);
//     lv_obj_align_to(btn_cancel, btn_manual, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
//     lv_obj_set_style_radius(btn_cancel, 10, 0);
//     lv_obj_set_style_pad_all(btn_cancel, 2, 0);
//     lv_obj_add_state(btn_cancel, LV_STATE_CHECKED);
//     lv_obj_t *lbl_cancel = lv_label_create(btn_cancel);
//     lv_obj_set_style_text_font(lbl_cancel, &lv_font_chinese_16, 0);
//     lv_label_set_text(lbl_cancel, "跳过");
//     lv_obj_center(lbl_cancel);
//     lv_obj_add_event_cb(
//         btn_cancel, [](lv_event_t *e)
//         {
//         if (e->code == LV_EVENT_CLICKED)
//         {
//             manual_config_ssid = NULL;
//             manual_config_password = NULL;          //标志用户选择跳过
//             xEventGroupSetBits(s_dpp_event_group, DPP_AUTH_CANCEL_BIT);          //通知DPP配网取消
//         } },
//         LV_EVENT_CLICKED, NULL);
//     hal.UNLOCKLV();
// }

// static void event_handler(void *arg, esp_event_base_t event_base,
//                           int32_t event_id, void *event_data)
// {
//     if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
//     {
//         ESP_ERROR_CHECK(esp_supp_dpp_start_listen());
//         ESP_LOGI(TAG, "Started listening for DPP Authentication");
//     }
//     else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
//     {
//         if (s_retry_num < 5)
//         {
//             esp_wifi_connect();
//             s_retry_num++;
//             ESP_LOGI(TAG, "retry to connect to the AP");
//         }
//         else
//         {
//             xEventGroupSetBits(s_dpp_event_group, DPP_CONNECT_FAIL_BIT);
//         }
//         ESP_LOGE(TAG, "connect to the AP fail");
//     }
//     else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
//     {
//         ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
//         ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
//         s_retry_num = 0;
//         xEventGroupSetBits(s_dpp_event_group, DPP_CONNECTED_BIT);
//     }
// }

// void dpp_enrollee_event_cb(esp_supp_dpp_event_t event, void *data)
// {
//     switch (event)
//     {
//     case ESP_SUPP_DPP_URI_READY:
//         if (data != NULL)
//         {
//             drawDPPQRCode((const char *)data);
//         }
//         break;
//     case ESP_SUPP_DPP_CFG_RECVD:
//         memcpy(&s_dpp_wifi_config, data, sizeof(s_dpp_wifi_config));
//         esp_wifi_set_config((wifi_interface_t)ESP_IF_WIFI_STA, &s_dpp_wifi_config);
//         s_retry_num = 0;
//         GUI::toast("正在连接");
//         xEventGroupSetBits(s_dpp_event_group, DPP_CONNECTED_BIT);
//         break;
//     case ESP_SUPP_DPP_FAIL:
//         if (s_retry_num < 5)
//         {
//             GUI::toast("配网失败，重试");
//             ESP_ERROR_CHECK(esp_supp_dpp_start_listen());
//             s_retry_num++;
//         }
//         else
//         {
//             xEventGroupSetBits(s_dpp_event_group, DPP_AUTH_FAIL_BIT);
//         }
//         break;
//     default:
//         break;
//     }
// }
// static void stop_dpp_and_clean(esp_netif_t *my_sta)
// {
//     esp_supp_dpp_stop_listen();
//     esp_supp_dpp_deinit();
//     ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
//     ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
//     vEventGroupDelete(s_dpp_event_group);
//     esp_wifi_stop();
//     delay(10);
//     esp_wifi_deinit();
//     // esp_netif_destroy(my_sta);
// }
// extern void add_esp_interface_netif(esp_interface_t interface, esp_netif_t *esp_netif);
// int esp_dpp_start(char *ssid, char *password)
// {
//     int result = -1;
//     s_dpp_event_group = xEventGroupCreate();
//     manual_config_sem = xSemaphoreCreateBinary();
//     xSemaphoreGive(manual_config_sem);
//     manual_config_ssid = ssid;
//     manual_config_password = password;

//     ESP_ERROR_CHECK(esp_netif_init());

//     esp_event_loop_create_default();
//     esp_netif_t *old_sta = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
//     esp_netif_destroy_default_wifi(old_sta);
//     esp_netif_t *my_sta = esp_netif_create_default_wifi_sta();
//     add_esp_interface_netif(ESP_IF_WIFI_STA, my_sta);
//     ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
//     ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     ESP_ERROR_CHECK(esp_wifi_init(&cfg));

//     ESP_ERROR_CHECK(esp_supp_dpp_init(dpp_enrollee_event_cb));
//     /* Currently only supported method is QR Code */
//     ESP_ERROR_CHECK(esp_supp_dpp_bootstrap_gen(EXAMPLE_DPP_LISTEN_CHANNEL_LIST, DPP_BOOTSTRAP_QR_CODE,
//                                                EXAMPLE_DPP_BOOTSTRAPPING_KEY, EXAMPLE_DPP_DEVICE_INFO));

//     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
//     ESP_ERROR_CHECK(esp_wifi_start());
//     EventBits_t bits = xEventGroupWaitBits(s_dpp_event_group,
//                                            DPP_CONNECTED_BIT | DPP_CONNECT_FAIL_BIT | DPP_AUTH_FAIL_BIT | DPP_AUTH_CANCEL_BIT,
//                                            pdFALSE,
//                                            pdFALSE,
//                                            portMAX_DELAY);
//     if (bits & DPP_CONNECTED_BIT)
//     {
//         result = 0;
//         strcpy(ssid, (const char *)s_dpp_wifi_config.sta.ssid);
//         strcpy(password, (const char *)s_dpp_wifi_config.sta.password);
//         vSemaphoreDelete(manual_config_sem);
//         stop_dpp_and_clean(my_sta);
//         WiFi.mode(WIFI_MODE_STA);
//         WiFi.begin(ssid, password);
//         if (WiFi.waitForConnectResult(5000) == WL_CONNECTED)
//         {
//             GUI::toast("成功");
//             result = 0;
//         }
//         else
//         {
//             GUI::toast("连接失败");
//             WiFi.disconnect(true);
//             result = -1;
//         }
//         return result;
//     }
//     stop_dpp_and_clean(my_sta);
//     if (bits & DPP_AUTH_CANCEL_BIT)
//     {
//         result = -2;
//         if (manual_config_ssid != NULL)
//         {
//             vTaskDelay(1);
//             xSemaphoreTake(manual_config_sem, portMAX_DELAY);
//             if (manual_config_ssid != NULL)
//             {
//                 WiFi.begin(manual_config_ssid, manual_config_password);
//                 if (WiFi.waitForConnectResult(15000) == WL_CONNECTED)
//                 {
//                     GUI::toast("成功");
//                     result = 0;
//                 }
//                 else
//                 {
//                     GUI::toast("连接失败");
//                     WiFi.disconnect(true);
//                     result = -1;
//                 }
//             }
//             else
//             {
//                 result = -2;
//                 GUI::toast("用户已取消");
//             }
//         }
//         else
//         {
//             GUI::toast("用户已取消");
//         }
//     }
//     else
//     {
//         result = -1;
//         GUI::toast("失败");
//     }
//     vSemaphoreDelete(manual_config_sem);

//     return result;
// }
int esp_dpp_start(char *ssid, char *password)
{
    int result = -1;
    manual_config_sem = xSemaphoreCreateBinary();
    manual_config_ssid = ssid;
    manual_config_password = password;
    hal.LOCKLV();
    load_scr_manual();
    hal.UNLOCKLV();
    vTaskDelay(500);
    xSemaphoreTake(manual_config_sem, portMAX_DELAY);
    if (manual_config_ssid != NULL)
    {
        WiFi.begin(manual_config_ssid, manual_config_password);
        if (WiFi.waitForConnectResult(15000) == WL_CONNECTED)
        {
            GUI::toast(_tr(I18N_ID_SUCCESS));
            result = 0;
        }
        else
        {
            GUI::toast(_tr(I18N_ID_CONNECT_FAILED));
            WiFiMgr.remove(manual_config_ssid);
            WiFi.disconnect(true, true);
            result = -1;
        }
    }
    else
    {
        result = -2;
        GUI::toast(_tr(I18N_ID_USER_CANCELLED));
    }

    vSemaphoreDelete(manual_config_sem);
    return result;
}