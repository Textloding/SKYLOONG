#include "A_Config.h"
#include <stdio.h>
#include <stdlib.h>

extern "C" const lv_font_t font_main1_time;
///////////////////////////////////////////////////////////////////////////
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
        lv_obj_set_pos(lbl_time, 150 + 8, 30);
        lv_obj_set_style_text_color(lbl_time, lv_color_hex(0xf9c262), 0);

        lbl_date = lv_label_create(home_appScreen);
        lv_label_set_text(lbl_date, "");
        lv_obj_set_style_text_font(lbl_date, &lv_font_montserrat_26, 0);
        lv_obj_set_style_text_color(lbl_date, lv_color_hex(0xf9c262), 0);
        lv_obj_set_pos(lbl_date, 6, 110);
        lv_obj_set_width(lbl_date, 90);
        lv_obj_set_style_text_align(lbl_date, LV_TEXT_ALIGN_CENTER, 0);
        img_am = lv_img_create(home_appScreen);
        // lv_img_set_src(img_am, &img_am1);
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
            lv_label_set_text_fmt(lbl_time, "%02d:%02d", hour, hal.datetime.minute);
        }
        else
        {
            lv_obj_add_flag(img_am, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(lbl_time, "%02d:%02d", hal.datetime.hour, hal.datetime.minute);
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
        lv_obj_set_pos(lbl_time, 136, 20);
        lv_obj_set_size(lbl_time, 171, 42);
        lv_obj_set_style_text_align(lbl_time, LV_TEXT_ALIGN_CENTER, 0);
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
        // lv_img_set_src(img_am, &img_am2);
        lv_obj_set_pos(img_am, 144, 29);
        // lv_obj_add_flag(img_am, LV_OBJ_FLAG_HIDDEN);
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
            // lv_obj_clear_flag(img_am, LV_OBJ_FLAG_HIDDEN);
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

            lv_label_set_text_fmt(lbl_time, "%02d:%02d", hal.datetime.hour > 12 ? hal.datetime.hour - 12 : hal.datetime.hour, hal.datetime.minute);
        }
        else
        {
            // lv_obj_add_flag(img_am, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(lbl_time, "%02d:%02d", hal.datetime.hour, hal.datetime.minute);
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
        lv_label_set_text(lbl_time, "12:34");
        lv_obj_set_style_text_font(lbl_time, &lv_font_montserrat_48, 0);
        lv_obj_align(lbl_time, LV_ALIGN_CENTER, 0, 0);
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
            lv_label_set_text_fmt(lbl_time, "%02d:%02d", hour, hal.datetime.minute);
        }
        else
        {
            lv_label_set_text_fmt(lbl_time, "%02d:%02d", hal.datetime.hour, hal.datetime.minute);
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

uint8_t last_minute = -1;
void AppHome::setup()
{
    last_minute = -1;
    hal.LOCKLV();
    if (hal.config_theme == 2)
        theme_fox::setup(_appScreen);
    else if (hal.config_theme == 1)
        theme_spartan::setup(_appScreen);
    else
        theme_default::setup(_appScreen);
    if (hal.config_theme == 2)
        theme_fox::loop();
    else if (hal.config_theme == 1)
        theme_spartan::loop();
    else
        theme_default::loop();
    if (hal.config_theme == 2)
        theme_fox::blinker();
    else if (hal.config_theme == 1)
        theme_spartan::blinker();
    else
        theme_default::blinker();
    hal.UNLOCKLV();
}
bool last_is_blinking = false;
extern SemaphoreHandle_t status_changed;
void AppHome::loop()
{
    hal.getTime();
    if (hal.datetime.minute != last_minute || xSemaphoreTake(status_changed, 500) == pdTRUE)
    {
        last_minute = hal.datetime.minute;
        hal.LOCKLV();
        if (hal.config_theme == 2)
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
        if (hal.config_theme == 2)
            theme_fox::blinker();
        else if (hal.config_theme == 1)
            theme_spartan::blinker();
        else
            theme_default::blinker();
        hal.UNLOCKLV();
    }
    delay(100);
}

void AppHome::destroy()
{
}
