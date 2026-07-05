#include "A_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define CENTER_X_1 84
#define CENTER_Y_1 140
#define CENTER_X_2 236
#define CENTER_Y_2 141
#define RADIUS 48
#define LINE_GAP 19
#define LINE_WIDTH 2
#define INITIAL_ANGLE 145
#define END_ANGLE 395
extern "C" const lv_img_dsc_t meter_bg;

static lv_obj_t *line_indicator_cpu;
static lv_obj_t *line_indicator_ram;
lv_obj_t *lbl_userdata;
static lv_point_t line_points_1[] = {{0, 0}, {0, 0}};
static lv_point_t line_points_2[] = {{0, 0}, {0, 0}};
static void meter_set_value(void *indic, int32_t pct)
{
    int32_t v = INITIAL_ANGLE + (pct * (END_ANGLE - INITIAL_ANGLE)) / 100;
    lv_obj_t *line = (lv_obj_t *)indic;
    lv_point_t *pt = (lv_point_t *)((lv_line_t *)indic)->point_array;
    lv_point_t p;
    p.x = RADIUS + (int32_t)(cos((v) * 3.14159265 / 180) * RADIUS);
    p.y = RADIUS + (int32_t)(sin((v) * 3.14159265 / 180) * RADIUS);
    pt[1] = p;
    p.x = RADIUS + (int32_t)(cos((v) * 3.14159265 / 180) * LINE_GAP);
    p.y = RADIUS + (int32_t)(sin((v) * 3.14159265 / 180) * LINE_GAP);
    pt[0] = p;
    lv_line_set_points(line, pt, 2);
    lv_obj_t *lbl = (lv_obj_t *)lv_obj_get_user_data(line);
    char buf[4];
    if (pct < 0 || pct > 99)
        pct = 99;
    sprintf(buf, "%"PRIu32, pct);
    lv_label_set_text(lbl, buf);
}
lv_obj_t *create_meter_indic(lv_obj_t *_appScreen, lv_point_t *line_points, int16_t x, int16_t y)
{
    lv_obj_t *line_indicator = lv_line_create(_appScreen);
    lv_obj_set_size(line_indicator, RADIUS * 2, RADIUS * 2);
    lv_obj_set_pos(line_indicator, x - RADIUS, y - RADIUS);
    lv_obj_set_style_line_width(line_indicator, LINE_WIDTH, 0);
    lv_obj_set_style_line_rounded(line_indicator, true, 0);
    lv_obj_set_style_line_color(line_indicator, lv_palette_main(LV_PALETTE_RED), 0);
    lv_line_set_points(line_indicator, line_points, 2);
    lv_obj_t *lbl_indicator = lv_label_create(_appScreen);
    lv_obj_set_style_text_font(lbl_indicator, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(lbl_indicator, x - 12, y - 11);
    lv_label_set_text(lbl_indicator, "--");
    lv_obj_set_user_data(line_indicator, lbl_indicator);
    return line_indicator;
}
uint32_t enter_millis = 0;
void AppSysinfo::setup()
{
    hal.LOCKLV();
    lv_obj_set_style_bg_img_src(_appScreen, &meter_bg, 0);
    line_indicator_cpu = create_meter_indic(_appScreen, line_points_1, CENTER_X_1, CENTER_Y_1);
    line_indicator_ram = create_meter_indic(_appScreen, line_points_2, CENTER_X_2, CENTER_Y_2);
    lbl_userdata = lv_label_create(_appScreen);
    lv_obj_set_size(lbl_userdata, 120, 38);
    lv_label_set_long_mode(lbl_userdata, LV_LABEL_LONG_WRAP);
    lv_obj_align(lbl_userdata, LV_ALIGN_TOP_MID, 0, 34);
    lv_obj_set_style_text_align(lbl_userdata, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(lbl_userdata, "Disconnected");
    lv_obj_set_style_text_font(lbl_userdata, &lv_font_chinese_16, 0);
    lv_obj_set_style_border_opa(lbl_userdata, LV_OPA_100, 0);
    lv_obj_set_style_border_color(lbl_userdata, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_border_width(lbl_userdata, 1, 0);
    lv_obj_set_style_radius(lbl_userdata, LV_RADIUS_CIRCLE, 0);
    hal.UNLOCKLV();
    enter_millis = millis();
    GUI::toast(_tr(I18N_ID_CONNECT_TO_WIFI_IN_5S));
    hal.sysinfo_update = true;
}
struct tcp_client_data
{
    float cpu_pct;
    float mem_pct;
};
static bool tcp_started = false;
static struct tcp_client_data s_tcp_client_data;
static WiFiClient tcpClient; // 用于获取CPU或内存数据
#include <lwip/sockets.h>
static bool handleTCPClient(WiFiClient &wifi)
{
    if (wifi.connected())
    {
        int32_t len = wifi.available();
        if (len > 0)
        {
            wifi.readBytes((char *)&s_tcp_client_data, sizeof(s_tcp_client_data));
			hal.LOCKLV();
            meter_set_value(line_indicator_cpu, s_tcp_client_data.cpu_pct * 100);
            meter_set_value(line_indicator_ram, s_tcp_client_data.mem_pct * 100);
			hal.UNLOCKLV();
            wifi.write(0x01);
        }
        return true;
    }
    else
        return false;
}

void TCPConnect(WiFiClient &wifi, const char *ip, const uint16_t port, bool &tcp_started)
{
    if (tcp_started)
    {
        IPAddress remote;
        remote.fromString(ip);
        ESP_LOGI("TCP", "Connecting to %s:%d", ip, port);
        ESP_LOGI("TCP", "%s", remote.toString().c_str());
        if (wifi.connect(remote, port) == false)
        {
            lwip_close(wifi.fd());
            wifi.stop();
            GUI::toast(_tr(I18N_ID_TCP_FAILED));
            tcp_started = false;
        }
    }
}

void TCPDisconnect(WiFiClient &wifi, bool &tcp_started)
{
    if (tcp_started)
    {
        lwip_close(wifi.fd());
        wifi.stop();
        tcp_started = false;
    }
}

void AppSysinfo::loop()
{
    if (!hal.sysinfo_enable) {
        xSemaphoreGive(appManagerLite._binary_switchApp);
        return;
    }

    if (hal.sysinfo_update == false) {
        hal.sysinfo_update = true;

        if (tcp_started) {
            TCPDisconnect(tcpClient, tcp_started);
        }

        enter_millis = millis() - 4000;
    }

    if (tcp_started)
    {
        if (WiFi.isConnected() == false || handleTCPClient(tcpClient) == false)
        {
            TCPDisconnect(tcpClient, tcp_started);
			hal.LOCKLV();
            lv_label_set_text(lbl_userdata, "Disconnected"); 
			hal.UNLOCKLV();
            GUI::toast(_tr(I18N_ID_OFFLINE_MODE));
        }
    }
    if (enter_millis != 0 && millis() - enter_millis > 5000)
    {
        GUI::toast(_tr(I18N_ID_CONNECTING));
        if (WiFiMgr.autoConnectSaved(6000))
        {
            if (hal.NTPSync()) {
                hal.time_sync = true;
            }

            tcp_started = true;
            TCPConnect(tcpClient, app_settings_save.remote_ip, app_settings_save.remote_port, tcp_started);
            if (handleTCPClient(tcpClient)) {
                tcp_started = true;
				hal.LOCKLV();
                lv_label_set_text(lbl_userdata, app_settings_save.userdata);
				hal.UNLOCKLV();
            } else {
				hal.LOCKLV();
               lv_label_set_text(lbl_userdata, "Disconnected");
			   hal.UNLOCKLV();
            }
        }
        else
        {
			hal.LOCKLV();
            lv_label_set_text(lbl_userdata, "Disconnected");
			hal.UNLOCKLV();
            GUI::toast(_tr(I18N_ID_CONNECT_FAILED));
        }
        enter_millis = 0;
    }
}

void AppSysinfo::destroy()
{
    TCPDisconnect(tcpClient, tcp_started);
    if (hal.server_started == false)
        WiFi.disconnect(true);
}
