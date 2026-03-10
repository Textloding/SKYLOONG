#include "A_Config.h"
#include <stdio.h>
#include <stdlib.h>

static lv_point_t qrcode_line_points[] = {{63, 3}, {256, 3}, {256, 196}, {63, 196}, {63, 3}};

void AppQRCode::setup() {
    char data[100];

    WiFi.begin();
    if (WiFi.waitForConnectResult(3000) == WL_CONNECTED) {
        if (hal.NTPSync()) {
            hal.time_sync = true;
        }
    }
    if (WiFi.getMode() == WIFI_OFF || (WiFi.getMode() == WIFI_STA && WiFi.isConnected() == false))
    {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("SKYLOONG 4.0 Screen");
    }
    hal.start_webserver();

    hal.LOCKLV();

    if (WiFi.getMode() == WIFI_AP) {
        lv_obj_t *lbl_prompt = lv_label_create(_appScreen);
        lv_obj_set_style_text_align(lbl_prompt, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_font(lbl_prompt, &lv_font_chinese_16, false);
        lv_obj_set_style_text_color(lbl_prompt, lv_color_white(), 0);
        lv_obj_set_size(lbl_prompt, 300, 30);
        snprintf(data, sizeof(data), _tr(I18N_ID_CONNECT_SCREEN_AP), "SKYLOONG 4.0 Screen");
        lv_label_set_text(lbl_prompt, data);
        lv_obj_set_pos(lbl_prompt, 10, 100);
    } else {
        lv_obj_t *line = lv_line_create(_appScreen);
        lv_obj_set_style_line_width(line, 1, 0);
        lv_obj_set_style_line_color(line, lv_color_white(), 0);
        lv_line_set_points(line, qrcode_line_points, 5);

        lv_obj_t *qrcode_obj = lv_qrcode_create(_appScreen, 190, lv_color_black(), lv_color_white());
        lv_obj_set_pos(qrcode_obj, 65, 5);
        snprintf(data, sizeof(data), "http://%s", WiFi.localIP().toString().c_str());
        lv_qrcode_update(qrcode_obj, data, strlen(data));

        lv_obj_t *lbl_prompt = lv_label_create(_appScreen);
        lv_obj_set_style_text_align(lbl_prompt, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_font(lbl_prompt, &lv_font_chinese_16, false);
        lv_obj_set_style_text_color(lbl_prompt, lv_color_white(), 0);
        lv_obj_set_size(lbl_prompt, 300, 20);
        snprintf(data, sizeof(data), _tr(I18N_ID_CONNECTED_WIFI), WiFi.SSID().c_str());
        lv_label_set_text(lbl_prompt, data);
        lv_obj_set_pos(lbl_prompt, 10, 197);

        lv_obj_t *lbl_url = lv_label_create(_appScreen);
        lv_obj_set_style_text_align(lbl_url, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_font(lbl_url, &lv_font_chinese_16, false);
        lv_obj_set_style_text_color(lbl_url, lv_color_white(), 0);
        lv_obj_set_size(lbl_url, 300, 20);
        snprintf(data, sizeof(data), "IP: %s", WiFi.localIP().toString().c_str());
        lv_label_set_text(lbl_url, data);
        lv_obj_set_pos(lbl_url, 10, 215);
    }

    hal.UNLOCKLV();
}

void AppQRCode::loop() {
    if (hal.config_wifi) {
        WiFiMgr.add(hal.ssid, hal.password);
        WiFi.disconnect(true);
        WiFi.mode(WIFI_STA);
        WiFi.begin(hal.ssid, hal.password);
        GUI::toast(_tr(I18N_ID_CONNECTING));
        if (WiFi.waitForConnectResult(4000) == WL_CONNECTED) { 
            if (hal.NTPSync()) {
                hal.time_sync = true;
            }
            hal.send_sysctl(EVENT_EXIT_QRCODE);
        } else {
            GUI::toast(_tr(I18N_ID_CONNECT_FAILED));
            WiFiMgr.remove(hal.ssid);
            WiFi.mode(WIFI_AP);
            WiFi.softAP("SKYLOONG 4.0 Screen");
        }
        hal.config_wifi = false;
    }
}

void AppQRCode::destroy() {
}
