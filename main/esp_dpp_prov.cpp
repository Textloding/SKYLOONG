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

wifi_config_t s_dpp_wifi_config;

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
    hal.lv_has_kb = false;
}
#include <WiFi.h>
#include <vector>
struct my_wifi_info_t
{
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
    for (int i = 0; i < n; ++i)
    {
        tmp.ssid = WiFi.SSID(i);
        tmp.rssi = WiFi.RSSI(i);
        tmp.auth = WiFi.encryptionType(i);
        wifi_list.push_back(tmp);
    }
    std::sort(wifi_list.begin(), wifi_list.end(), [](const struct my_wifi_info_t &a, const struct my_wifi_info_t &b)
              { return a.rssi > b.rssi; });
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
        if (wifi_list[i].auth != WIFI_AUTH_OPEN)
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
        lv_obj_add_event_cb(
            btn, [](lv_event_t *e)
            {
                if (lv_indev_get_key(lv_indev_get_act()) == LV_KEY_ESC)
                {
                    hal.lv_has_kb = false;
                    cancel_connect();
                } },
            LV_EVENT_KEY, NULL);
        lv_obj_add_event_cb(
            btn, [](lv_event_t *e)
            { lv_obj_scroll_to_view(lv_event_get_target(e), LV_ANIM_OFF); },
            LV_EVENT_FOCUSED, NULL);
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
    while (1)
    {
        if (xSemaphoreTake(manual_config_sem, 100))
            break;
        if (hal.setting_mode == false)
        {
            cancel_connect();
            break;
        }
    }
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