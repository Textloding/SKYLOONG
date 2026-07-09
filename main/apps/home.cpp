#include "A_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" const lv_font_t font_main1_time;

namespace theme_fox
{
    extern "C" const lv_img_dsc_t img_main1_bg;
    extern "C" const lv_img_dsc_t battery_frame_1;
    extern "C" const lv_img_dsc_t battery_charging_1;
    extern "C" const lv_img_dsc_t img_bt1_1;
    extern "C" const lv_img_dsc_t img_bt1_2;
    extern "C" const lv_img_dsc_t img_bt2_1;
    extern "C" const lv_img_dsc_t img_bt2_2;
    extern "C" const lv_img_dsc_t img_caplock1_1;
    extern "C" const lv_img_dsc_t img_caplock1_2;
    extern "C" const lv_img_dsc_t img_mac1_1;
    extern "C" const lv_img_dsc_t img_mac1_2;
    extern "C" const lv_img_dsc_t img_numlock1_1;
    extern "C" const lv_img_dsc_t img_numlock1_2;
    extern "C" const lv_img_dsc_t img_rf1_1;
    extern "C" const lv_img_dsc_t img_rf1_2;
    extern "C" const lv_img_dsc_t img_sllock1_1;
    extern "C" const lv_img_dsc_t img_sllock1_2;
    extern "C" const lv_img_dsc_t img_usb1_1;
    extern "C" const lv_img_dsc_t img_usb1_2;
    extern "C" const lv_img_dsc_t img_win1_1;
    extern "C" const lv_img_dsc_t img_win1_2;
    extern "C" const lv_img_dsc_t img_winlock1_1;
    extern "C" const lv_img_dsc_t img_winlock1_2;
    extern "C" const lv_img_dsc_t img_am1;
    extern "C" const lv_img_dsc_t img_pm1;
    lv_obj_t *img_battery_frame;
    lv_obj_t *img_battery_charging;
    lv_obj_t *obj_battery_cell;
    lv_obj_t *lbl_battery;
    lv_obj_t *img_1;
    lv_obj_t *img_2;
    lv_obj_t *img_3;
    lv_obj_t *img_4;
    lv_obj_t *img_5;
    lv_obj_t *img_6;
    lv_obj_t *img_am;
    lv_obj_t *lbl_time;
    lv_obj_t *lbl_date;
    bool toggle_connection;
    void setup(lv_obj_t *home_appScreen)
    {
        toggle_connection = true;
        lv_obj_set_style_bg_img_src(home_appScreen, &img_main1_bg, 0);
        img_1 = lv_img_create(home_appScreen);
        lv_obj_set_pos(img_1, 115, 120);

        img_2 = lv_img_create(home_appScreen);
        lv_obj_set_pos(img_2, 187, 120);

        img_3 = lv_img_create(home_appScreen);
        lv_obj_set_pos(img_3, 259, 120);

        img_4 = lv_img_create(home_appScreen);
        lv_obj_set_pos(img_4, 115, 177);

        img_5 = lv_img_create(home_appScreen);
        lv_obj_set_pos(img_5, 187, 177);

        img_6 = lv_img_create(home_appScreen);
        lv_obj_set_pos(img_6, 259, 177);

        lbl_time = lv_label_create(home_appScreen);
        lv_label_set_text(lbl_time, "");
        lv_obj_set_style_text_font(lbl_time, &font_main1_time, 0);
        lv_obj_set_pos(lbl_time, 157, 30);
        lv_obj_set_size(lbl_time, 163, 42);
        lv_obj_set_style_text_align(lbl_time, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_style_text_color(lbl_time, lv_color_hex(0xf9c262), 0);

        lbl_date = lv_label_create(home_appScreen);
        lv_label_set_text(lbl_date, "");
        lv_obj_set_style_text_font(lbl_date, &lv_font_montserrat_26, 0);
        lv_obj_set_style_text_color(lbl_date, lv_color_hex(0xf9c262), 0);
        lv_obj_set_pos(lbl_date, 6, 110);
        lv_obj_set_width(lbl_date, 90);
        lv_obj_set_style_text_align(lbl_date, LV_TEXT_ALIGN_CENTER, 0);
        img_am = lv_img_create(home_appScreen);
        lv_obj_set_pos(img_am, 118 + 8, 50);
        lv_obj_add_flag(img_am, LV_OBJ_FLAG_HIDDEN);
        // 电池
        img_battery_frame = lv_img_create(home_appScreen);
        lv_img_set_src(img_battery_frame, &battery_frame_1);
        lv_obj_set_pos(img_battery_frame, 195, 82);
        obj_battery_cell = lv_obj_create(home_appScreen); // 最宽38
        lv_obj_set_pos(obj_battery_cell, 199, 85);
        lv_obj_set_size(obj_battery_cell, 38, 10);
        lv_obj_set_style_radius(obj_battery_cell, 2, 0);
        lv_obj_clear_flag(obj_battery_cell, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_border_opa(obj_battery_cell, 0, 0);
        lv_obj_set_style_bg_color(obj_battery_cell, lv_color_hex(0xf8c163), 0);
        img_battery_charging = lv_img_create(home_appScreen);
        lv_obj_set_pos(img_battery_charging, 185, 84);
        lv_img_set_src(img_battery_charging, &battery_charging_1);
        lv_obj_add_flag(img_battery_charging, LV_OBJ_FLAG_HIDDEN);
        lbl_battery = lv_label_create(home_appScreen);
        lv_obj_set_pos(lbl_battery, 173 - 9, 82);
        lv_obj_set_style_text_color(lbl_battery, lv_color_hex(0xf8c163), 0);
        lv_label_set_text(lbl_battery, "--%");
    }
    void blinker()
    {
        // img2_state
        if (hal.kb_status.chan_state == 1 || hal.kb_status.chan_state == 2)
        {
            if (toggle_connection)
            {
                toggle_connection = false;
                lv_obj_add_flag(img_2, LV_OBJ_FLAG_HIDDEN);
            }
            else
            {
                toggle_connection = true;
                lv_obj_clear_flag(img_2, LV_OBJ_FLAG_HIDDEN);
            }
        }
        else
        {
            lv_obj_clear_flag(img_2, LV_OBJ_FLAG_HIDDEN);
        }
        if (hal.kb_status.chan_state & 1)
        {
            if (hal.kb_status.channel_current == 0)
                lv_img_set_src(img_2, &img_rf1_1);
            else if (hal.kb_status.channel_current == 1)
                lv_img_set_src(img_2, &img_bt1_1);
            else if (hal.kb_status.channel_current == 2)
                lv_img_set_src(img_2, &img_bt2_1);
            else if (hal.kb_status.channel_current == 3)
                lv_img_set_src(img_2, &img_usb1_1);
        }
        else
        {
            if (hal.kb_status.channel_current == 0)
                lv_img_set_src(img_2, &img_rf1_2);
            else if (hal.kb_status.channel_current == 1)
                lv_img_set_src(img_2, &img_bt1_2);
            else if (hal.kb_status.channel_current == 2)
                lv_img_set_src(img_2, &img_bt2_2);
            else if (hal.kb_status.channel_current == 3)
                lv_img_set_src(img_2, &img_usb1_2);
        }
    }
    void loop()
    {
        if (hal.battery_pct != 0)
        {
            if (hal.battery_status == BATTERY_STATUS_CHARGING)
            {
                lv_obj_clear_flag(img_battery_charging, LV_OBJ_FLAG_HIDDEN);
                lv_label_set_text(lbl_battery, "");
                lv_obj_set_width(obj_battery_cell, 38);
            }
            else
            {
                lv_obj_add_flag(img_battery_charging, LV_OBJ_FLAG_HIDDEN);
                if (hal.battery_pct == 100)
                {
                    lv_obj_set_x(lbl_battery, 173 - 9 - 5);
                }
                else
                {
                    lv_obj_set_x(lbl_battery, 173 - 9);
                }
                lv_label_set_text_fmt(lbl_battery, "%d%%", hal.battery_pct);
                lv_obj_set_width(obj_battery_cell, hal.battery_pct * 38 / 100);
            }
        }
        if (hal.config_time_12hr)
        {
            int hour = hal.datetime.hour;
            if (hour > 12)
            {
                lv_img_set_src(img_am, &img_pm1);
                hour -= 12;
            }
            else
            {
                lv_img_set_src(img_am, &img_am1);
            }

            lv_obj_clear_flag(img_am, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(lbl_time, "%02d:%02d:%02d", hour, hal.datetime.minute, hal.datetime.second);
        }
        else
        {
            lv_obj_add_flag(img_am, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(lbl_time, "%02d:%02d:%02d", hal.datetime.hour, hal.datetime.minute, hal.datetime.second);
        }
        lv_label_set_text_fmt(lbl_date, "%02d/%02d", hal.datetime.month, hal.datetime.dayMonth);
        // img1
        if (hal.kb_status.system == 1)
            lv_img_set_src(img_1, &img_win1_1);
        else
            lv_img_set_src(img_1, &img_mac1_1);
        // img2
        blinker();
        // img3
        if (hal.kb_status.winlk && lv_img_get_src(img_3) != &img_winlock1_1)
            lv_img_set_src(img_3, &img_winlock1_1);
        else if (!hal.kb_status.winlk && lv_img_get_src(img_3) != &img_winlock1_2)
            lv_img_set_src(img_3, &img_winlock1_2);
        // img4
        if (hal.kb_status.capslock && lv_img_get_src(img_4) != &img_caplock1_1)
            lv_img_set_src(img_4, &img_caplock1_1);
        else if (!hal.kb_status.capslock && lv_img_get_src(img_4) != &img_caplock1_2)
            lv_img_set_src(img_4, &img_caplock1_2);
        // img5
        if (hal.kb_status.scrolllock && lv_img_get_src(img_5) != &img_sllock1_1)
            lv_img_set_src(img_5, &img_sllock1_1);
        else if (!hal.kb_status.scrolllock && lv_img_get_src(img_5) != &img_sllock1_2)
            lv_img_set_src(img_5, &img_sllock1_2);
        // img6
        if (hal.kb_status.numlock && lv_img_get_src(img_6) != &img_numlock1_1)
            lv_img_set_src(img_6, &img_numlock1_1);
        else if (!hal.kb_status.numlock && lv_img_get_src(img_6) != &img_numlock1_2)
            lv_img_set_src(img_6, &img_numlock1_2);
    }
}
namespace theme_spartan
{
    extern "C" const lv_img_dsc_t img_main2_bg;
    extern "C" const lv_img_dsc_t battery_frame_2;
    extern "C" const lv_img_dsc_t battery_indicator_2;
    extern "C" const lv_img_dsc_t img_bt1;
    extern "C" const lv_img_dsc_t img_bt2;
    extern "C" const lv_img_dsc_t img_caplock;
    extern "C" const lv_img_dsc_t img_mac;
    extern "C" const lv_img_dsc_t img_numlock;
    extern "C" const lv_img_dsc_t img_rf;
    extern "C" const lv_img_dsc_t img_sl;
    extern "C" const lv_img_dsc_t img_usb;
    extern "C" const lv_img_dsc_t img_win;
    extern "C" const lv_img_dsc_t img_winlock;
    extern "C" const lv_img_dsc_t img_am2;
    extern "C" const lv_img_dsc_t img_pm2;
    extern "C" const lv_img_dsc_t charge_bolt_spartan;
    lv_obj_t *img_battery_frame;
    lv_obj_t *img_battery_charging;
    lv_obj_t *obj_battery_cell;
    lv_obj_t *lbl_battery;
    lv_obj_t *img_battery_bolt;
    lv_obj_t *obj_img_1;
    lv_obj_t *obj_img_2;
    lv_obj_t *obj_img_3;
    lv_obj_t *obj_img_4;
    lv_obj_t *obj_img_5;
    lv_obj_t *obj_img_6;
    lv_obj_t *img_1;
    lv_obj_t *img_2;
    lv_obj_t *img_3;
    lv_obj_t *img_4;
    lv_obj_t *img_5;
    lv_obj_t *img_6;
    lv_obj_t *img_am;
    lv_obj_t *lbl_time;
    lv_obj_t *lbl_date;
    lv_obj_t *lbl_year;
    bool toggle_connection;
    const lv_img_dsc_t *img_2_src[4] = {&img_rf, &img_bt1, &img_bt2, &img_usb};
    void setup(lv_obj_t *home_appScreen)
    {
        toggle_connection = true;
        lv_obj_set_style_bg_img_src(home_appScreen, &img_main2_bg, 0);
        obj_img_1 = lv_obj_create(home_appScreen);
        lv_obj_set_size(obj_img_1, 45, 45);
        lv_obj_set_pos(obj_img_1, 138, 128);
        lv_obj_set_style_bg_opa(obj_img_1, LV_OPA_70, 0);
        lv_obj_set_style_border_opa(obj_img_1, 0, 0);
        lv_obj_set_style_radius(obj_img_1, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_1, LV_OBJ_FLAG_SCROLLABLE);
        img_1 = lv_img_create(obj_img_1);
        lv_img_set_src(img_1, &img_win);
        lv_obj_center(img_1);

        obj_img_2 = lv_obj_create(home_appScreen);
        lv_obj_set_size(obj_img_2, 45, 45);
        lv_obj_set_pos(obj_img_2, 198, 128);
        lv_obj_set_style_bg_opa(obj_img_2, LV_OPA_70, 0);
        lv_obj_set_style_border_opa(obj_img_2, 0, 0);
        lv_obj_set_style_radius(obj_img_2, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_2, LV_OBJ_FLAG_SCROLLABLE);
        img_2 = lv_img_create(obj_img_2);
        lv_obj_center(img_2);

        obj_img_3 = lv_obj_create(home_appScreen);
        lv_obj_set_size(obj_img_3, 45, 45);
        lv_obj_set_pos(obj_img_3, 258, 128);
        lv_obj_set_style_bg_opa(obj_img_3, LV_OPA_70, 0);
        lv_obj_set_style_border_opa(obj_img_3, 0, 0);
        lv_obj_set_style_radius(obj_img_3, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_3, LV_OBJ_FLAG_SCROLLABLE);
        img_3 = lv_img_create(obj_img_3);
        lv_img_set_src(img_3, &img_winlock);
        lv_obj_center(img_3);

        obj_img_4 = lv_obj_create(home_appScreen);
        lv_obj_set_size(obj_img_4, 45, 45);
        lv_obj_set_pos(obj_img_4, 138, 182);
        lv_obj_set_style_bg_opa(obj_img_4, LV_OPA_70, 0);
        lv_obj_set_style_border_opa(obj_img_4, 0, 0);
        lv_obj_set_style_radius(obj_img_4, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_4, LV_OBJ_FLAG_SCROLLABLE);
        img_4 = lv_img_create(obj_img_4);
        lv_img_set_src(img_4, &img_caplock);
        lv_obj_center(img_4);

        obj_img_5 = lv_obj_create(home_appScreen);
        lv_obj_set_size(obj_img_5, 45, 45);
        lv_obj_set_pos(obj_img_5, 198, 182);
        lv_obj_set_style_bg_opa(obj_img_5, LV_OPA_70, 0);
        lv_obj_set_style_border_opa(obj_img_5, 0, 0);
        lv_obj_set_style_radius(obj_img_5, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_5, LV_OBJ_FLAG_SCROLLABLE);
        img_5 = lv_img_create(obj_img_5);
        lv_img_set_src(img_5, &img_sl);
        lv_obj_center(img_5);

        obj_img_6 = lv_obj_create(home_appScreen);
        lv_obj_set_size(obj_img_6, 45, 45);
        lv_obj_set_pos(obj_img_6, 258, 182);
        lv_obj_set_style_bg_opa(obj_img_6, LV_OPA_70, 0);
        lv_obj_set_style_border_opa(obj_img_6, 0, 0);
        lv_obj_set_style_radius(obj_img_6, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_6, LV_OBJ_FLAG_SCROLLABLE);
        img_6 = lv_img_create(obj_img_6);
        lv_img_set_src(img_6, &img_numlock);
        lv_obj_center(img_6);

        lbl_time = lv_label_create(home_appScreen);
        lv_label_set_text(lbl_time, "");
        lv_obj_set_style_text_font(lbl_time, &font_main1_time, 0);
        lv_obj_set_pos(lbl_time, 168, 20);
        lv_obj_set_size(lbl_time, 152, 42);
        lv_obj_set_style_text_align(lbl_time, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_style_text_color(lbl_time, lv_color_hex(0x000000), 0);

        lbl_date = lv_label_create(home_appScreen);
        lv_label_set_text(lbl_date, "");
        lv_obj_set_style_text_font(lbl_date, &lv_font_montserrat_28, 0);
        lv_obj_set_style_text_color(lbl_date, lv_color_hex(0x000000), 0);
        lv_obj_set_pos(lbl_date, 17, 41);
        lv_obj_set_width(lbl_date, 90);
        lv_obj_set_style_text_align(lbl_date, LV_TEXT_ALIGN_CENTER, 0);

        lbl_year = lv_label_create(home_appScreen);
        lv_label_set_text(lbl_year, "");
        lv_obj_set_style_text_font(lbl_year, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(lbl_year, lv_color_hex(0x3c3c3c), 0);
        lv_obj_set_pos(lbl_year, 12, 17);
        lv_obj_set_width(lbl_year, 100);
        lv_obj_set_style_text_align(lbl_year, LV_TEXT_ALIGN_CENTER, 0);
        img_am = lv_img_create(home_appScreen);
        lv_obj_set_pos(img_am, 144, 29);
        img_battery_frame = lv_img_create(home_appScreen);
        lv_obj_set_pos(img_battery_frame, 250 - 50, 58 + 5);
        lv_img_set_src(img_battery_frame, &battery_frame_2);
        obj_battery_cell = lv_obj_create(home_appScreen);
        lv_obj_set_pos(obj_battery_cell, 261 - 50, 63 + 5);
        lv_obj_set_size(obj_battery_cell, 29, 5);
        lv_obj_set_style_radius(obj_battery_cell, 0, 0);
        lv_obj_clear_flag(obj_battery_cell, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_border_opa(obj_battery_cell, 0, 0);
        lv_obj_set_style_bg_opa(obj_battery_cell, 0, 0);
        lv_obj_set_style_bg_img_src(obj_battery_cell, &battery_indicator_2, 0);
        lbl_battery = lv_label_create(home_appScreen);
        lv_obj_set_pos(lbl_battery, 250 - 50 - 22 - 9, 58 + 5);
        lv_obj_set_style_text_color(lbl_battery, lv_color_hex(0xFFFFFF), 0);
        lv_label_set_text(lbl_battery, "98%");
        img_battery_bolt = lv_img_create(home_appScreen);
        lv_obj_set_pos(img_battery_bolt, 250 - 50 - 10, 58 + 5);
        lv_img_set_src(img_battery_bolt, &charge_bolt_spartan);
        lv_obj_add_flag(img_battery_bolt, LV_OBJ_FLAG_HIDDEN);
    }
    void blinker()
    {
        lv_img_set_src(img_2, img_2_src[hal.kb_status.channel_current]);
        if (hal.kb_status.chan_state == 1 || hal.kb_status.chan_state == 2)
        {
            if (toggle_connection)
            {
                toggle_connection = false;
                lv_obj_add_flag(img_2, LV_OBJ_FLAG_HIDDEN);
            }
            else
            {
                toggle_connection = true;
                lv_obj_clear_flag(img_2, LV_OBJ_FLAG_HIDDEN);
            }
        }
        else
        {
            lv_obj_clear_flag(img_2, LV_OBJ_FLAG_HIDDEN);
        }
        if (hal.kb_status.chan_state == 3)
        {
            lv_obj_set_style_bg_color(obj_img_2, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
        }
        else
        {
            lv_obj_set_style_bg_color(obj_img_2, lv_color_hex(0x282b30), 0);
        }
    }
    void loop()
    {
        if (hal.battery_pct != 0)
        {
            if (hal.battery_status != BATTERY_STATUS_CHARGING)
            {
                if (hal.battery_pct == 100)
                {
                    lv_obj_set_x(lbl_battery, 250 - 50 - 28 - 9);
                }
                else
                {
                    lv_obj_set_x(lbl_battery, 250 - 50 - 22 - 9);
                }
                lv_label_set_text_fmt(lbl_battery, "%d%%", hal.battery_pct);
                lv_obj_add_flag(img_battery_bolt, LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_width(obj_battery_cell, hal.battery_pct * 29 / 100);
            }
            else
            {
                lv_obj_clear_flag(img_battery_bolt, LV_OBJ_FLAG_HIDDEN);
                lv_label_set_text(lbl_battery, "");
                lv_obj_set_width(obj_battery_cell, 29);
            }
        }
        if (hal.config_time_12hr)
        {
            int hour = hal.datetime.hour;
            if (hour > 12)
            {
                lv_img_set_src(img_am, &img_pm2);
                lv_obj_set_pos(img_am, 144, 43);
                hour -= 12;
            }
            else
            {
                lv_img_set_src(img_am, &img_am2);
                lv_obj_set_pos(img_am, 144, 29);
            }

            lv_label_set_text_fmt(lbl_time, "%02d:%02d:%02d", hal.datetime.hour > 12 ? hal.datetime.hour - 12 : hal.datetime.hour, hal.datetime.minute, hal.datetime.second);
        }
        else
        {
            lv_label_set_text_fmt(lbl_time, "%02d:%02d:%02d", hal.datetime.hour, hal.datetime.minute, hal.datetime.second);
        }
        lv_label_set_text_fmt(lbl_date, "%02d/%02d", hal.datetime.month, hal.datetime.dayMonth);
        lv_label_set_text_fmt(lbl_year, "%04d", hal.datetime.year);
        blinker();
        if (hal.kb_status.capslock && lv_obj_get_style_bg_color(obj_img_4, 0).full != lv_palette_main(LV_PALETTE_BLUE_GREY).full)
            lv_obj_set_style_bg_color(obj_img_4, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
        else if (!hal.kb_status.capslock && lv_obj_get_style_bg_color(obj_img_4, 0).full != lv_color_hex(0x282b30).full)
            lv_obj_set_style_bg_color(obj_img_4, lv_color_hex(0x282b30), 0);

        if (hal.kb_status.numlock && lv_obj_get_style_bg_color(obj_img_6, 0).full != lv_palette_main(LV_PALETTE_BLUE_GREY).full)
            lv_obj_set_style_bg_color(obj_img_6, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
        else if (!hal.kb_status.numlock && lv_obj_get_style_bg_color(obj_img_6, 0).full != lv_color_hex(0x282b30).full)
            lv_obj_set_style_bg_color(obj_img_6, lv_color_hex(0x282b30), 0);

        if (hal.kb_status.scrolllock && lv_obj_get_style_bg_color(obj_img_5, 0).full != lv_palette_main(LV_PALETTE_BLUE_GREY).full)
            lv_obj_set_style_bg_color(obj_img_5, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
        else if (!hal.kb_status.scrolllock && lv_obj_get_style_bg_color(obj_img_5, 0).full != lv_color_hex(0x282b30).full)
            lv_obj_set_style_bg_color(obj_img_5, lv_color_hex(0x282b30), 0);
        if (hal.kb_status.system == 1 && lv_img_get_src(img_1) != &img_win)
            lv_img_set_src(img_1, &img_win);
        else if (hal.kb_status.system == 0 && lv_img_get_src(img_1) != &img_mac)
            lv_img_set_src(img_1, &img_mac);
        if (hal.kb_status.winlk && lv_obj_get_style_bg_color(obj_img_3, 0).full != lv_palette_main(LV_PALETTE_BLUE_GREY).full)
            lv_obj_set_style_bg_color(obj_img_3, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
        else if (!hal.kb_status.winlk && lv_obj_get_style_bg_color(obj_img_3, 0).full != lv_color_hex(0x282b30).full)
            lv_obj_set_style_bg_color(obj_img_3, lv_color_hex(0x282b30), 0);
    }

} // namespace theme_spartan
namespace theme_default
{
    extern "C" const lv_img_dsc_t img_main2_bg;
    extern "C" const lv_img_dsc_t img_bt1;
    extern "C" const lv_img_dsc_t img_bt2;
    extern "C" const lv_img_dsc_t img_caplock;
    extern "C" const lv_img_dsc_t img_mac;
    extern "C" const lv_img_dsc_t img_numlock;
    extern "C" const lv_img_dsc_t img_rf;
    extern "C" const lv_img_dsc_t img_sl;
    extern "C" const lv_img_dsc_t img_usb;
    extern "C" const lv_img_dsc_t img_win;
    extern "C" const lv_img_dsc_t img_winlock;
    extern "C" const lv_img_dsc_t img_am2;
    extern "C" const lv_img_dsc_t img_pm2;
    lv_obj_t *obj_img_1;
    lv_obj_t *obj_img_2;
    lv_obj_t *obj_img_3;
    lv_obj_t *obj_img_4;
    lv_obj_t *obj_img_5;
    lv_obj_t *obj_img_6;
    lv_obj_t *img_1;
    lv_obj_t *img_2;
    lv_obj_t *img_3;
    lv_obj_t *img_4;
    lv_obj_t *img_5;
    lv_obj_t *img_6;
    lv_obj_t *img_am;
    lv_obj_t *lbl_time;
    lv_obj_t *lbl_date;
    lv_obj_t *lbl_year;
    lv_obj_t *lbl_battery;
    lv_obj_t *arc_battery;
    bool toggle_connection;
    void setup(lv_obj_t *home_appScreen)
    {
        toggle_connection = true;
        lv_obj_t *box_upper_left = lv_obj_create(home_appScreen);
        lv_obj_set_size(box_upper_left, 180, 110);
        lv_obj_set_pos(box_upper_left, 10, 10);
        lv_obj_set_style_bg_color(box_upper_left, lv_palette_main(LV_PALETTE_BLUE), 0);

        lbl_time = lv_label_create(box_upper_left);
        lv_label_set_text(lbl_time, "12:34:56");
        lv_obj_set_style_text_font(lbl_time, &lv_font_montserrat_40, 0);
        lv_obj_align(lbl_time, LV_ALIGN_CENTER, 0, -8);
        img_am = lv_img_create(box_upper_left);
        lv_obj_align_to(img_am, lbl_time, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
        lv_obj_add_flag(img_am, LV_OBJ_FLAG_HIDDEN);

        lv_obj_t *box_upper_right = lv_obj_create(home_appScreen);
        lv_obj_set_size(box_upper_right, 110, 110);
        lv_obj_set_pos(box_upper_right, 200, 10);

        arc_battery = lv_arc_create(box_upper_right);
        lv_arc_set_bg_angles(arc_battery, 0, 270);
        lv_arc_set_angles(arc_battery, 0, 0);
        lv_obj_set_size(arc_battery, lv_pct(100), lv_pct(100));
        lv_obj_align(arc_battery, LV_ALIGN_CENTER, 0, 0);
        lv_arc_set_rotation(arc_battery, 135);
        lv_obj_set_style_opa(arc_battery, 0, LV_PART_KNOB);
        lv_arc_set_value(arc_battery, 50);
        lbl_battery = lv_label_create(box_upper_right);
        lv_label_set_text(lbl_battery, "50%");
        lv_obj_align(lbl_battery, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_font(lbl_battery, &lv_font_chinese_16, 0);

        lv_obj_t *box_lower_left = lv_obj_create(home_appScreen);
        lv_obj_set_style_pad_all(box_lower_left, 4, 0);
        lv_obj_set_size(box_lower_left, 130, 100);
        lv_obj_set_pos(box_lower_left, 10, 130);
        lbl_year = lv_label_create(box_lower_left);
        lv_label_set_text(lbl_year, "2024");
        lv_obj_set_style_text_font(lbl_year, &lv_font_montserrat_32, 0);
        lv_obj_align(lbl_year, LV_ALIGN_TOP_LEFT, 0, 0);
        lbl_date = lv_label_create(box_lower_left);
        lv_label_set_text(lbl_date, "12/24");
        lv_obj_set_style_text_font(lbl_date, &lv_font_montserrat_40, 0);
        lv_obj_align(lbl_date, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

        lv_obj_t *box_lower_right = lv_obj_create(home_appScreen);
        lv_obj_set_size(box_lower_right, 160, 100);
        lv_obj_set_pos(box_lower_right, 150, 130);
        lv_obj_set_style_pad_all(box_lower_right, 2, 0);

        obj_img_1 = lv_obj_create(box_lower_right);
        lv_obj_set_size(obj_img_1, 41, 41);
        lv_obj_set_pos(obj_img_1, 0, 0);
        lv_obj_set_style_bg_color(obj_img_1, lv_color_hex(0x5dc289), 0);
        lv_obj_set_style_border_opa(obj_img_1, 0, 0);
        lv_obj_set_style_radius(obj_img_1, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_1, LV_OBJ_FLAG_SCROLLABLE);
        img_1 = lv_img_create(obj_img_1);
        lv_img_set_src(img_1, &img_win);
        lv_obj_center(img_1);

        obj_img_2 = lv_obj_create(box_lower_right);
        lv_obj_set_size(obj_img_2, 41, 41);
        lv_obj_align(obj_img_2, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_bg_color(obj_img_2, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_color(obj_img_2, lv_color_hex(0x603bff), LV_STATE_USER_1);
        lv_obj_set_style_bg_color(obj_img_2, lv_palette_main(LV_PALETTE_GREEN), LV_STATE_USER_2);
        lv_obj_set_style_border_opa(obj_img_2, 0, 0);
        lv_obj_set_style_radius(obj_img_2, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_2, LV_OBJ_FLAG_SCROLLABLE);
        img_2 = lv_img_create(obj_img_2);
        lv_img_set_src(img_2, &img_bt1);
        lv_obj_center(img_2);

        obj_img_3 = lv_obj_create(box_lower_right);
        lv_obj_set_size(obj_img_3, 41, 41);
        lv_obj_align(obj_img_3, LV_ALIGN_TOP_RIGHT, 0, 0);
        lv_obj_set_style_bg_color(obj_img_3, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_color(obj_img_3, lv_color_hex(0xd796ff), LV_STATE_USER_1);
        lv_obj_set_style_border_opa(obj_img_3, 0, 0);
        lv_obj_set_style_radius(obj_img_3, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_3, LV_OBJ_FLAG_SCROLLABLE);
        img_3 = lv_img_create(obj_img_3);
        lv_img_set_src(img_3, &img_winlock);
        lv_obj_center(img_3);

        obj_img_4 = lv_obj_create(box_lower_right);
        lv_obj_set_size(obj_img_4, 41, 41);
        lv_obj_align(obj_img_4, LV_ALIGN_BOTTOM_LEFT, 0, 0);
        lv_obj_set_style_bg_color(obj_img_4, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_color(obj_img_4, lv_color_hex(0xffd48f), LV_STATE_USER_1);
        lv_obj_set_style_border_opa(obj_img_4, 0, 0);
        lv_obj_set_style_radius(obj_img_4, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_4, LV_OBJ_FLAG_SCROLLABLE);
        img_4 = lv_img_create(obj_img_4);
        lv_img_set_src(img_4, &img_caplock);
        lv_obj_center(img_4);

        obj_img_5 = lv_obj_create(box_lower_right);
        lv_obj_set_size(obj_img_5, 41, 41);
        lv_obj_align(obj_img_5, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_style_bg_color(obj_img_5, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_color(obj_img_5, lv_color_hex(0xd95f64), LV_STATE_USER_1);
        lv_obj_set_style_border_opa(obj_img_5, 0, 0);
        lv_obj_set_style_radius(obj_img_5, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_5, LV_OBJ_FLAG_SCROLLABLE);
        img_5 = lv_img_create(obj_img_5);
        lv_img_set_src(img_5, &img_sl);
        lv_obj_center(img_5);

        obj_img_6 = lv_obj_create(box_lower_right);
        lv_obj_set_size(obj_img_6, 41, 41);
        lv_obj_align(obj_img_6, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
        lv_obj_set_style_bg_color(obj_img_6, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_color(obj_img_6, lv_color_hex(0x5f8ed9), LV_STATE_USER_1);
        lv_obj_set_style_border_opa(obj_img_6, 0, 0);
        lv_obj_set_style_radius(obj_img_6, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_6, LV_OBJ_FLAG_SCROLLABLE);
        img_6 = lv_img_create(obj_img_6);
        lv_img_set_src(img_6, &img_numlock);
        lv_obj_center(img_6);
    }
    void blinker()
    {
        if (hal.kb_status.chan_state == 1 || hal.kb_status.chan_state == 2)
        {
            if (toggle_connection)
            {
                toggle_connection = false;
                lv_obj_add_flag(img_2, LV_OBJ_FLAG_HIDDEN);
            }
            else
            {
                toggle_connection = true;
                lv_obj_clear_flag(img_2, LV_OBJ_FLAG_HIDDEN);
            }
        }
        else
        {
            lv_obj_clear_flag(img_2, LV_OBJ_FLAG_HIDDEN);
        }
        if (hal.kb_status.chan_state == 1)
        {
            lv_obj_clear_state(obj_img_2, LV_STATE_USER_2);
            lv_obj_add_state(obj_img_2, LV_STATE_USER_1);
        }
        else if (hal.kb_status.chan_state == 3)
        {
            // 已连接
            lv_obj_clear_state(obj_img_2, LV_STATE_USER_1);
            lv_obj_add_state(obj_img_2, LV_STATE_USER_2);
        }
        else
        {
            lv_obj_clear_state(obj_img_2, LV_STATE_USER_1);
            lv_obj_clear_state(obj_img_2, LV_STATE_USER_2);
        }
        if (hal.kb_status.channel_current == 0)
            lv_img_set_src(img_2, &img_rf);
        else if (hal.kb_status.channel_current == 1)
            lv_img_set_src(img_2, &img_bt1);
        else if (hal.kb_status.channel_current == 2)
            lv_img_set_src(img_2, &img_bt2);
        else if (hal.kb_status.channel_current == 3)
            lv_img_set_src(img_2, &img_usb);
    }
    void loop()
    {
        if (hal.battery_pct != 0)
        {
            lv_arc_set_value(arc_battery, hal.battery_pct);
            lv_label_set_text_fmt(lbl_battery, "%d%%", hal.battery_pct);
            lv_obj_set_style_text_font(lbl_battery, &lv_font_chinese_16, 0);
            if (hal.battery_status == BATTERY_STATUS_CHARGED)
            {
                lv_obj_set_style_arc_color(arc_battery, lv_palette_main(LV_PALETTE_GREEN), LV_PART_INDICATOR);
                lv_label_set_text(lbl_battery, "100%");
            }
            else if (hal.battery_status == BATTERY_STATUS_CHARGING)
            {
                lv_obj_set_style_arc_color(arc_battery, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR);
                lv_obj_set_style_text_font(lbl_battery, &lv_font_montserrat_20, 0);
                lv_label_set_text(lbl_battery, LV_SYMBOL_CHARGE);
                lv_arc_set_value(arc_battery, 100);
            }
            else
            {
                if (hal.battery_pct < 20)
                    lv_obj_set_style_arc_color(arc_battery, lv_palette_main(LV_PALETTE_RED), LV_PART_INDICATOR);
                else
                    lv_obj_set_style_arc_color(arc_battery, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR);
            }
        }
        if (hal.config_time_12hr)
        {
            int hour = hal.datetime.hour;
            if (hour > 12)
            {
                lv_img_set_src(img_am, &img_pm2);
                hour -= 12;
            }
            else
            {
                lv_img_set_src(img_am, &img_am2);
            }

            lv_obj_clear_flag(img_am, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(lbl_time, "%02d:%02d:%02d", hour, hal.datetime.minute, hal.datetime.second);
        }
        else
        {
            lv_label_set_text_fmt(lbl_time, "%02d:%02d:%02d", hal.datetime.hour, hal.datetime.minute, hal.datetime.second);
            lv_obj_add_flag(img_am, LV_OBJ_FLAG_HIDDEN);
        }
        lv_label_set_text_fmt(lbl_date, "%02d/%02d", hal.datetime.month, hal.datetime.dayMonth);
        lv_label_set_text_fmt(lbl_year, "%04d", hal.datetime.year);
        if (hal.kb_status.system == 1)
            lv_img_set_src(img_1, &img_win);
        else
            lv_img_set_src(img_1, &img_mac);
        blinker();
        if (hal.kb_status.winlk && lv_obj_has_state(obj_img_3, LV_STATE_USER_1) == false)
            lv_obj_add_state(obj_img_3, LV_STATE_USER_1);
        else if (hal.kb_status.winlk == false && lv_obj_has_state(obj_img_3, LV_STATE_USER_1))
            lv_obj_clear_state(obj_img_3, LV_STATE_USER_1);

        if (hal.kb_status.capslock && lv_obj_has_state(obj_img_4, LV_STATE_USER_1) == false)
            lv_obj_add_state(obj_img_4, LV_STATE_USER_1);
        else if (hal.kb_status.capslock == false && lv_obj_has_state(obj_img_4, LV_STATE_USER_1))
            lv_obj_clear_state(obj_img_4, LV_STATE_USER_1);

        if (hal.kb_status.scrolllock && lv_obj_has_state(obj_img_5, LV_STATE_USER_1) == false)
            lv_obj_add_state(obj_img_5, LV_STATE_USER_1);
        else if (hal.kb_status.scrolllock == false && lv_obj_has_state(obj_img_5, LV_STATE_USER_1))
            lv_obj_clear_state(obj_img_5, LV_STATE_USER_1);

        if (hal.kb_status.numlock && lv_obj_has_state(obj_img_6, LV_STATE_USER_1) == false)
            lv_obj_add_state(obj_img_6, LV_STATE_USER_1);
        else if (hal.kb_status.numlock == false && lv_obj_has_state(obj_img_6, LV_STATE_USER_1))
            lv_obj_clear_state(obj_img_6, LV_STATE_USER_1);
    }

} // namespace theme_default
namespace theme_space
{
    extern "C" const lv_img_dsc_t img_space_bg;
    extern "C" const lv_img_dsc_t * const img_space_taikonaut_frames[10];
    extern "C" const lv_img_dsc_t img_bt1;
    extern "C" const lv_img_dsc_t img_bt2;
    extern "C" const lv_img_dsc_t img_caplock;
    extern "C" const lv_img_dsc_t img_mac;
    extern "C" const lv_img_dsc_t img_numlock;
    extern "C" const lv_img_dsc_t img_rf;
    extern "C" const lv_img_dsc_t img_sl;
    extern "C" const lv_img_dsc_t img_usb;
    extern "C" const lv_img_dsc_t img_win;
    extern "C" const lv_img_dsc_t img_winlock;

    struct HomeWeatherData
    {
        char location[32];
        uint8_t weatherCode_today;
        uint8_t weatherCode_tomorrow;
        uint8_t weatherCode_tomorrow_1;
        int8_t temperature_now;
        char index_wear[16];
        char index_sport[16];
        char index_flu[16];
        char index_car[16];
    };

    lv_obj_t *lbl_time;
    lv_obj_t *lbl_date;
    lv_obj_t *lbl_week;
    lv_obj_t *lbl_lunar;
    lv_obj_t *lbl_location;
    lv_obj_t *lbl_weather_cond;
    lv_obj_t *lbl_weather_temp;
    lv_obj_t *lbl_weather_meta;
    lv_obj_t *img_astronaut;
    lv_obj_t *status_cells[6];
    lv_obj_t *status_icons[6];
    HomeWeatherData weather_cache;
    bool weather_loaded;
    bool toggle_connection;
    uint8_t anim_frame;
    uint32_t last_anim_ms;
    uint32_t last_weather_ms;
    const lv_img_dsc_t *channel_icons[4] = {&img_rf, &img_bt1, &img_bt2, &img_usb};

    const char *weekday_name(uint8_t day)
    {
        static const char *names[] = {
            "星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日",
        };
        if (day <= 7)
            return names[day];
        return "星期日";
    }

    const char *weather_name(uint8_t code)
    {
        static const char *names[] = {
            "晴", "晴", "晴", "晴", "多云", "晴间多云", "晴间多云", "大部多云", "大部多云", "阴",
            "阵雨", "雷阵雨", "冰雹", "小雨", "中雨", "大雨", "暴雨", "大暴雨", "特大暴雨", "冻雨",
            "雨夹雪", "阵雪", "小雪", "中雪", "大雪", "暴雪", "浮尘", "扬沙", "沙尘暴", "强沙尘暴",
            "雾", "霾", "风", "大风", "飓风", "热带风暴", "龙卷风", "冷", "热", "未知",
        };
        if (code < sizeof(names) / sizeof(names[0]))
            return names[code];
        return "未知";
    }

    static const uint32_t lunar_info[] = {
        0x04bd8, 0x04ae0, 0x0a570, 0x054d5, 0x0d260, 0x0d950, 0x16554, 0x056a0, 0x09ad0, 0x055d2,
        0x04ae0, 0x0a5b6, 0x0a4d0, 0x0d250, 0x1d255, 0x0b540, 0x0d6a0, 0x0ada2, 0x095b0, 0x14977,
        0x04970, 0x0a4b0, 0x0b4b5, 0x06a50, 0x06d40, 0x1ab54, 0x02b60, 0x09570, 0x052f2, 0x04970,
        0x06566, 0x0d4a0, 0x0ea50, 0x06e95, 0x05ad0, 0x02b60, 0x186e3, 0x092e0, 0x1c8d7, 0x0c950,
        0x0d4a0, 0x1d8a6, 0x0b550, 0x056a0, 0x1a5b4, 0x025d0, 0x092d0, 0x0d2b2, 0x0a950, 0x0b557,
        0x06ca0, 0x0b550, 0x15355, 0x04da0, 0x0a5d0, 0x14573, 0x052d0, 0x0a9a8, 0x0e950, 0x06aa0,
        0x0aea6, 0x0ab50, 0x04b60, 0x0aae4, 0x0a570, 0x05260, 0x0f263, 0x0d950, 0x05b57, 0x056a0,
        0x096d0, 0x04dd5, 0x04ad0, 0x0a4d0, 0x0d4d4, 0x0d250, 0x0d558, 0x0b540, 0x0b6a0, 0x195a6,
        0x095b0, 0x049b0, 0x0a974, 0x0a4b0, 0x0b27a, 0x06a50, 0x06d40, 0x0af46, 0x0ab60, 0x09570,
        0x04af5, 0x04970, 0x064b0, 0x074a3, 0x0ea50, 0x06b58, 0x055c0, 0x0ab60, 0x096d5, 0x092e0,
        0x0c960, 0x0d954, 0x0d4a0, 0x0da50, 0x07552, 0x056a0, 0x0abb7, 0x025d0, 0x092d0, 0x0cab5,
        0x0a950, 0x0b4a0, 0x0baa4, 0x0ad50, 0x055d9, 0x04ba0, 0x0a5b0, 0x15176, 0x052b0, 0x0a930,
        0x07954, 0x06aa0, 0x0ad50, 0x05b52, 0x04b60, 0x0a6e6, 0x0a4e0, 0x0d260, 0x0ea65, 0x0d530,
        0x05aa0, 0x076a3, 0x096d0, 0x04bd7, 0x04ad0, 0x0a4d0, 0x1d0b6, 0x0d250, 0x0d520, 0x0dd45,
        0x0b5a0, 0x056d0, 0x055b2, 0x049b0, 0x0a577, 0x0a4b0, 0x0aa50, 0x1b255, 0x06d20, 0x0ada0,
        0x14b63, 0x09370, 0x049f8, 0x04970, 0x064b0, 0x168a6, 0x0ea50, 0x06b20, 0x1a6c4, 0x0aae0,
        0x0a2e0, 0x0d2e3, 0x0c960, 0x0d557, 0x0d4a0, 0x0da50, 0x05d55, 0x056a0, 0x0a6d0, 0x055d4,
        0x052d0, 0x0a9b8, 0x0a950, 0x0b4a0, 0x0b6a6, 0x0ad50, 0x055a0, 0x0aba4, 0x0a5b0, 0x052b0,
        0x0b273, 0x06930, 0x07337, 0x06aa0, 0x0ad50, 0x14b55, 0x04b60, 0x0a570, 0x054e4, 0x0d160,
        0x0e968, 0x0d520, 0x0daa0, 0x16aa6, 0x056d0, 0x04ae0, 0x0a9d4, 0x0a2d0, 0x0d150, 0x0f252,
        0x0d520,
    };

    int solar_day_number(int y, int m, int d)
    {
        if (m <= 2)
        {
            y--;
            m += 12;
        }
        return 365 * y + y / 4 - y / 100 + y / 400 + (153 * (m - 3) + 2) / 5 + d - 1;
    }

    int lunar_year_days(int year)
    {
        int sum = 348;
        uint32_t info = lunar_info[year - 1900];
        for (uint32_t mask = 0x8000; mask > 0x8; mask >>= 1)
            if (info & mask)
                sum++;
        return sum + ((info & 0xf) ? ((info & 0x10000) ? 30 : 29) : 0);
    }

    int lunar_leap_month(int year)
    {
        return lunar_info[year - 1900] & 0xf;
    }

    int lunar_month_days(int year, int month)
    {
        return (lunar_info[year - 1900] & (0x10000 >> month)) ? 30 : 29;
    }

    void lunar_day_name(int day, char *out, size_t out_size)
    {
        static const char *n[] = {"", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十"};
        if (day == 10)
            snprintf(out, out_size, "初十");
        else if (day == 20)
            snprintf(out, out_size, "二十");
        else if (day == 30)
            snprintf(out, out_size, "三十");
        else
            snprintf(out, out_size, "%s%s", day < 10 ? "初" : (day < 20 ? "十" : (day < 30 ? "廿" : "卅")), n[day % 10]);
    }

    void lunar_label(uint16_t year, uint8_t month, uint8_t day, char *out, size_t out_size)
    {
        if (year < 1900 || year > 2100)
        {
            snprintf(out, out_size, "农历");
            return;
        }
        int offset = solar_day_number(year, month, day) - solar_day_number(1900, 1, 31);
        int lunar_year = 1900;
        while (lunar_year <= 2100)
        {
            int days = lunar_year_days(lunar_year);
            if (offset < days)
                break;
            offset -= days;
            lunar_year++;
        }

        int leap = lunar_leap_month(lunar_year);
        bool is_leap = false;
        int lunar_month = 1;
        while (lunar_month <= 12)
        {
            int days = is_leap ? ((lunar_info[lunar_year - 1900] & 0x10000) ? 30 : 29) : lunar_month_days(lunar_year, lunar_month);
            if (offset < days)
                break;
            offset -= days;
            if (leap == lunar_month && !is_leap)
                is_leap = true;
            else
            {
                is_leap = false;
                lunar_month++;
            }
        }

        static const char *month_names[] = {"", "正", "二", "三", "四", "五", "六", "七", "八", "九", "十", "冬", "腊"};
        char day_buf[12];
        lunar_day_name(offset + 1, day_buf, sizeof(day_buf));
        snprintf(out, out_size, "农历%s%s月%s", is_leap ? "闰" : "", month_names[lunar_month], day_buf);
    }

    lv_obj_t *make_label(lv_obj_t *parent, int16_t x, int16_t y, int16_t w, const lv_font_t *font, lv_color_t color)
    {
        lv_obj_t *label = lv_label_create(parent);
        lv_obj_set_pos(label, x, y);
        lv_obj_set_width(label, w);
        lv_obj_set_style_text_font(label, font, 0);
        lv_obj_set_style_text_color(label, color, 0);
        lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0);
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
        return label;
    }

    lv_obj_t *create_status_cell(lv_obj_t *parent, int16_t x, int16_t y, const lv_img_dsc_t *icon)
    {
        lv_obj_t *cell = lv_obj_create(parent);
        lv_obj_set_pos(cell, x, y);
        lv_obj_set_size(cell, 34, 31);
        lv_obj_set_style_pad_all(cell, 0, 0);
        lv_obj_set_style_radius(cell, 0, 0);
        lv_obj_set_style_bg_opa(cell, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_opa(cell, LV_OPA_TRANSP, 0);
        lv_obj_set_style_shadow_opa(cell, LV_OPA_TRANSP, 0);
        lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t *img = lv_img_create(cell);
        lv_img_set_src(img, icon);
        lv_obj_center(img);
        return cell;
    }

    void set_status_fill(lv_obj_t *cell, bool active, lv_color_t color)
    {
        if (active)
        {
            lv_obj_set_style_bg_color(cell, color, 0);
            lv_obj_set_style_bg_opa(cell, LV_OPA_90, 0);
        }
        else
        {
            lv_obj_set_style_bg_opa(cell, LV_OPA_TRANSP, 0);
        }
    }

    void load_weather_cache(bool force)
    {
        if (!force && millis() - last_weather_ms < 60000)
            return;
        last_weather_ms = millis();
        File f = LittleFS.open(".weather", "r");
        if (!f)
        {
            weather_loaded = false;
            return;
        }
        weather_loaded = f.readBytes((char *)&weather_cache, sizeof(weather_cache)) == sizeof(weather_cache);
        f.close();
        if (weather_cache.weatherCode_today >= 40)
            weather_cache.weatherCode_today = 39;
    }

    void update_weather_labels()
    {
        load_weather_cache(false);
        if (!weather_loaded)
        {
            lv_label_set_text(lbl_location, "暂无天气");
            lv_label_set_text(lbl_weather_cond, "--");
            lv_label_set_text(lbl_weather_temp, "--°");
            lv_label_set_text(lbl_weather_meta, "天气同步中");
            return;
        }
        lv_label_set_text(lbl_location, strlen(weather_cache.location) > 0 ? weather_cache.location : "当前位置");
        lv_label_set_text(lbl_weather_cond, weather_name(weather_cache.weatherCode_today));
        lv_label_set_text_fmt(lbl_weather_temp, "%d°", weather_cache.temperature_now);
        if (strlen(weather_cache.index_wear) > 0)
            lv_label_set_text_fmt(lbl_weather_meta, "穿衣 %s", weather_cache.index_wear);
        else
            lv_label_set_text(lbl_weather_meta, "数据已同步");
    }

    void setup(lv_obj_t *home_appScreen)
    {
        anim_frame = 0;
        last_anim_ms = 0;
        last_weather_ms = 0;
        weather_loaded = false;
        toggle_connection = true;
        lv_obj_set_style_bg_color(home_appScreen, lv_color_hex(0x030815), 0);
        lv_obj_set_style_bg_img_src(home_appScreen, &img_space_bg, 0);

        img_astronaut = lv_img_create(home_appScreen);
        lv_img_set_src(img_astronaut, img_space_taikonaut_frames[0]);
        lv_img_set_zoom(img_astronaut, 286);
        lv_obj_set_pos(img_astronaut, 238, -2);

        lbl_time = make_label(home_appScreen, 14, 36, 174, &lv_font_montserrat_32, lv_color_hex(0xffffff));
        lv_label_set_text(lbl_time, "00:00:00");
        lbl_week = make_label(home_appScreen, 17, 79, 66, &lv_font_chinese_16, lv_color_hex(0xfff1ce));
        lbl_lunar = make_label(home_appScreen, 83, 81, 116, &lv_font_chinese_16, lv_color_hex(0xf2f7ff));
        lbl_date = make_label(home_appScreen, 18, 106, 174, &lv_font_chinese_16, lv_color_hex(0x68e5ff));

        lbl_location = make_label(home_appScreen, 188, 86, 90, &lv_font_chinese_16, lv_color_hex(0x8cecff));
        lv_obj_set_style_text_align(lbl_location, LV_TEXT_ALIGN_LEFT, 0);
        lbl_weather_cond = make_label(home_appScreen, 202, 112, 58, &lv_font_chinese_16, lv_color_hex(0xffffff));
        lbl_weather_temp = make_label(home_appScreen, 235, 105, 70, &lv_font_montserrat_28, lv_color_hex(0xffffff));
        lbl_weather_meta = make_label(home_appScreen, 203, 150, 106, &lv_font_chinese_16, lv_color_hex(0xd4e4ff));

        const int16_t start_x = 26;
        const int16_t y = 204;
        const int16_t gap = 12;
        status_cells[0] = create_status_cell(home_appScreen, start_x + 0 * (34 + gap), y, &img_win);
        status_cells[1] = create_status_cell(home_appScreen, start_x + 1 * (34 + gap), y, &img_rf);
        status_cells[2] = create_status_cell(home_appScreen, start_x + 2 * (34 + gap), y, &img_winlock);
        status_cells[3] = create_status_cell(home_appScreen, start_x + 3 * (34 + gap), y, &img_caplock);
        status_cells[4] = create_status_cell(home_appScreen, start_x + 4 * (34 + gap), y, &img_sl);
        status_cells[5] = create_status_cell(home_appScreen, start_x + 5 * (34 + gap), y, &img_numlock);
        for (uint8_t i = 0; i < 6; i++)
            status_icons[i] = lv_obj_get_child(status_cells[i], 0);

        load_weather_cache(true);
        update_weather_labels();
    }

    void animate()
    {
        if (img_astronaut == NULL || millis() - last_anim_ms < 90)
            return;
        static const int8_t bob_x[] = {0, 2, 3, 2, 0, -2, -3, -2};
        static const int8_t bob_y[] = {0, -2, -3, -2, 0, 2, 3, 2};
        last_anim_ms = millis();
        anim_frame = (anim_frame + 1) % 10;
        lv_img_set_src(img_astronaut, img_space_taikonaut_frames[anim_frame]);
        lv_obj_set_pos(img_astronaut, 238 + bob_x[anim_frame & 7], -2 + bob_y[anim_frame & 7]);
    }

    void blinker()
    {
        uint8_t channel = hal.kb_status.channel_current < 4 ? hal.kb_status.channel_current : 0;
        lv_img_set_src(status_icons[1], channel_icons[channel]);
        if (hal.kb_status.chan_state == 1 || hal.kb_status.chan_state == 2)
        {
            toggle_connection = !toggle_connection;
            if (toggle_connection)
                lv_obj_clear_flag(status_icons[1], LV_OBJ_FLAG_HIDDEN);
            else
                lv_obj_add_flag(status_icons[1], LV_OBJ_FLAG_HIDDEN);
            set_status_fill(status_cells[1], true, lv_color_hex(0x5865ff));
        }
        else
        {
            lv_obj_clear_flag(status_icons[1], LV_OBJ_FLAG_HIDDEN);
            set_status_fill(status_cells[1], hal.kb_status.chan_state == 3, lv_color_hex(0x22c7a6));
        }
        animate();
    }

    void loop()
    {
        int hour = hal.datetime.hour;
        if (hal.config_time_12hr)
        {
            if (hour > 12)
                hour -= 12;
            else if (hour == 0)
                hour = 12;
        }
        lv_label_set_text_fmt(lbl_time, "%02d:%02d:%02d", hour, hal.datetime.minute, hal.datetime.second);

        char date_buf[32];
        snprintf(date_buf, sizeof(date_buf), "%04d年%02d月%02d日", hal.datetime.year, hal.datetime.month, hal.datetime.dayMonth);
        lv_label_set_text(lbl_date, date_buf);
        lv_label_set_text(lbl_week, weekday_name(hal.datetime.dayWeek));
        char lunar_buf[48];
        lunar_label(hal.datetime.year, hal.datetime.month, hal.datetime.dayMonth, lunar_buf, sizeof(lunar_buf));
        lv_label_set_text(lbl_lunar, lunar_buf);

        if (hal.kb_status.system == 1)
            lv_img_set_src(status_icons[0], &img_win);
        else
            lv_img_set_src(status_icons[0], &img_mac);
        blinker();
        set_status_fill(status_cells[2], hal.kb_status.winlk, lv_color_hex(0x4cdbff));
        set_status_fill(status_cells[3], hal.kb_status.capslock, lv_color_hex(0xffcd4c));
        set_status_fill(status_cells[4], hal.kb_status.scrolllock, lv_color_hex(0xff7a66));
        set_status_fill(status_cells[5], hal.kb_status.numlock, lv_color_hex(0x8bf47f));
        update_weather_labels();
        animate();
    }

} // namespace theme_space

uint8_t last_second = 0xff;
uint8_t current_theme = 0;
void AppHome::setup()
{
    last_second = 0xff;
    hal.LOCKLV();
    if (hal.config_theme == 3)
        theme_space::setup(_appScreen);
    else if (hal.config_theme == 2)
        theme_fox::setup(_appScreen);
    else if (hal.config_theme == 1)
        theme_spartan::setup(_appScreen);
    else
        theme_default::setup(_appScreen);
    if (hal.config_theme == 3)
        theme_space::loop();
    else if (hal.config_theme == 2)
        theme_fox::loop();
    else if (hal.config_theme == 1)
        theme_spartan::loop();
    else
        theme_default::loop();
    if (hal.config_theme == 3)
        theme_space::blinker();
    else if (hal.config_theme == 2)
        theme_fox::blinker();
    else if (hal.config_theme == 1)
        theme_spartan::blinker();
    else
        theme_default::blinker();
    hal.UNLOCKLV();
    current_theme = hal.config_theme;
}
bool last_is_blinking = false;
extern SemaphoreHandle_t status_changed;
void AppHome::loop()
{
    if (current_theme != hal.config_theme) {
        hal.send_sysctl(EVENT_HOME_REFRESH);
        current_theme = hal.config_theme;
        return;
    }
    hal.getTime();
    TickType_t status_wait = hal.config_theme == 3 ? 0 : 500;
    if (hal.datetime.second != last_second || xSemaphoreTake(status_changed, status_wait) == pdTRUE)
    {
        last_second = hal.datetime.second;
        hal.LOCKLV();
        if (hal.config_theme == 3)
            theme_space::loop();
        else if (hal.config_theme == 2)
            theme_fox::loop();
        else if (hal.config_theme == 1)
            theme_spartan::loop();
        else
            theme_default::loop();
        hal.UNLOCKLV();
        return;
    }
    if (hal.kb_status.chan_state == 1 || hal.kb_status.chan_state == 2)
    {
        hal.LOCKLV();
        if (hal.config_theme == 3)
            theme_space::blinker();
        else if (hal.config_theme == 2)
            theme_fox::blinker();
        else if (hal.config_theme == 1)
            theme_spartan::blinker();
        else
            theme_default::blinker();
        hal.UNLOCKLV();
    }
    if (hal.config_theme == 3)
    {
        hal.LOCKLV();
        theme_space::animate();
        hal.UNLOCKLV();
    }
    delay(100);
}

void AppHome::destroy()
{
}
