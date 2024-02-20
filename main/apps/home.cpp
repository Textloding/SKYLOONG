#include "A_Config.h"
#include <stdio.h>
#include <stdlib.h>

/*
// APS 曲线部分代码
extern "C" const lv_img_dsc_t img_APS;

lv_obj_t *chart_aps = NULL;
lv_chart_series_t* ser1 = NULL;
lv_obj_t* lbl_aps = NULL;
lv_obj_t* lbl_apm = NULL;
lv_obj_t* lbl_keyN = NULL;
#include <math.h>
int key_total = 0;
int time_cnt = 0;
void set_aps(int aps)
{
    key_total += aps;
    time_cnt++;
    lv_label_set_text_fmt(lbl_aps, "%d", aps);
    lv_label_set_text_fmt(lbl_apm, "%d", key_total * 60 / time_cnt);
    lv_label_set_text_fmt(lbl_keyN, "%d", key_total);
    lv_chart_set_next_value(chart_aps, ser1, (int)(log10(aps) * 100));
}
void my_app_main()
{
    lv_obj_set_style_bg_img_src(home_appScreen, &img_APS, 0);
    lbl_aps = lv_label_create(home_appScreen);
    lv_label_set_text(lbl_aps, "0");
    lv_obj_set_style_text_align(lbl_aps, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(lbl_aps, 251, 70);
    lv_obj_set_size(lbl_aps, 50, 22);
    lv_obj_set_style_text_color(lbl_aps, lv_color_black(), 0);

    lbl_apm = lv_label_create(home_appScreen);
    lv_label_set_text(lbl_apm, "0");
    lv_obj_set_style_text_align(lbl_apm, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(lbl_apm, 251, 126);
    lv_obj_set_size(lbl_apm, 50, 22);
    lv_obj_set_style_text_color(lbl_apm, lv_color_black(), 0);

    lbl_keyN = lv_label_create(home_appScreen);
    lv_label_set_text(lbl_keyN, "0");
    lv_obj_set_style_text_align(lbl_keyN, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(lbl_keyN, 251, 188);
    lv_obj_set_size(lbl_keyN, 50, 22);
    lv_obj_set_style_text_color(lbl_keyN, lv_color_black(), 0);

    chart_aps = lv_chart_create(home_appScreen);
    lv_obj_set_style_bg_opa(chart_aps, LV_OPA_0, 0);
    lv_obj_set_style_border_opa(chart_aps, LV_OPA_0, 0);
    lv_obj_set_style_line_opa(chart_aps, LV_OPA_0, 0);
    lv_obj_set_pos(chart_aps, 39, 52);
    lv_obj_set_size(chart_aps, 193, 151);
    lv_chart_set_type(chart_aps, LV_CHART_TYPE_LINE);
    ser1 = lv_chart_add_series(chart_aps, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    lv_obj_set_style_pad_all(chart_aps, 0, 0);
    lv_chart_set_update_mode(chart_aps, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_range(chart_aps, LV_CHART_AXIS_PRIMARY_Y, 0, 247);
    set_aps(0);
    set_aps(10);
    set_aps(30);
    set_aps(20);
    set_aps(50);
    set_aps(100);
    set_aps(110);
    set_aps(160);
    set_aps(300);
}
*/
extern "C" const lv_font_t font_main1_time;
///////////////////////////////////////////////////////////////////////////
namespace theme_fox
{
    extern "C" const lv_img_dsc_t img_main1_bg;
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
        lv_obj_set_pos(lbl_time, 150, 30);
        lv_obj_set_style_text_color(lbl_time, lv_color_hex(0xf9c262), 0);

        lbl_date = lv_label_create(home_appScreen);
        lv_label_set_text(lbl_date, "");
        lv_obj_set_style_text_font(lbl_date, &lv_font_montserrat_26, 0);
        lv_obj_set_style_text_color(lbl_date, lv_color_hex(0xf9c262), 0);
        lv_obj_set_pos(lbl_date, 3, 110);
        lv_obj_set_width(lbl_date, 90);
        lv_obj_set_style_text_align(lbl_date, LV_TEXT_ALIGN_CENTER, 0);
        img_am = lv_img_create(home_appScreen);
        // lv_img_set_src(img_am, &img_am1);
        lv_obj_set_pos(img_am, 118, 50);
        lv_obj_add_flag(img_am, LV_OBJ_FLAG_HIDDEN);
    }
    void loop()
    {
        hal.getTime();
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
        // img3
        if (hal.kb_status.winlk)
            lv_img_set_src(img_3, &img_winlock1_1);
        else
            lv_img_set_src(img_3, &img_winlock1_2);
        // img4
        if (hal.kb_status.capslock)
            lv_img_set_src(img_4, &img_caplock1_1);
        else
            lv_img_set_src(img_4, &img_caplock1_2);
        // img5
        if (hal.kb_status.scrolllock)
            lv_img_set_src(img_5, &img_sllock1_1);
        else
            lv_img_set_src(img_5, &img_sllock1_2);
        // img6
        if (hal.kb_status.numlock)
            lv_img_set_src(img_6, &img_numlock1_1);
        else
            lv_img_set_src(img_6, &img_numlock1_2);
    }
}
namespace theme_spartan
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
        lv_obj_set_pos(lbl_time, 180, 25);
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
        lv_obj_set_pos(img_am, 282, 53);
        // lv_obj_add_flag(img_am, LV_OBJ_FLAG_HIDDEN);
    }
    void loop()
    {
        lv_img_set_src(img_2, img_2_src[hal.kb_status.channel_current]);
        hal.getTime();
        if (hal.config_time_12hr)
        {
            // lv_obj_clear_flag(img_am, LV_OBJ_FLAG_HIDDEN);
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

            lv_label_set_text_fmt(lbl_time, "%02d:%02d", hal.datetime.hour > 12 ? hal.datetime.hour - 12 : hal.datetime.hour, hal.datetime.minute);
        }
        else
        {
            // lv_obj_add_flag(img_am, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(lbl_time, "%02d:%02d", hal.datetime.hour, hal.datetime.minute);
        }
        lv_label_set_text_fmt(lbl_date, "%02d/%02d", hal.datetime.month, hal.datetime.dayMonth);
        lv_label_set_text_fmt(lbl_year, "%04d", hal.datetime.year);
        if (hal.kb_status.chan_state == 3)
        {
            lv_obj_set_style_bg_color(obj_img_2, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
        }
        else
        {
            lv_obj_set_style_bg_color(obj_img_2, lv_color_hex(0x282b30), 0);
        }
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
        if (hal.kb_status.capslock)
            lv_obj_set_style_bg_color(obj_img_4, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
        else
            lv_obj_set_style_bg_color(obj_img_4, lv_color_hex(0x282b30), 0);

        if (hal.kb_status.numlock)
            lv_obj_set_style_bg_color(obj_img_6, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
        else
            lv_obj_set_style_bg_color(obj_img_6, lv_color_hex(0x282b30), 0);

        if (hal.kb_status.scrolllock)
            lv_obj_set_style_bg_color(obj_img_5, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
        else
            lv_obj_set_style_bg_color(obj_img_5, lv_color_hex(0x282b30), 0);
        if (hal.kb_status.system == 1)
            lv_img_set_src(img_1, &img_win);
        else
            lv_img_set_src(img_1, &img_mac);
        if (hal.kb_status.winlk)
            lv_obj_set_style_bg_color(obj_img_3, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
        else
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
        lv_obj_set_size(obj_img_1, 45, 45);
        lv_obj_set_pos(obj_img_1, 0, 0);
        lv_obj_set_style_bg_color(obj_img_1, lv_palette_main(LV_PALETTE_GREEN), 0);
        lv_obj_set_style_border_opa(obj_img_1, 0, 0);
        lv_obj_set_style_radius(obj_img_1, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_1, LV_OBJ_FLAG_SCROLLABLE);
        img_1 = lv_img_create(obj_img_1);
        lv_img_set_src(img_1, &img_win);
        lv_obj_center(img_1);

        obj_img_2 = lv_obj_create(box_lower_right);
        lv_obj_set_size(obj_img_2, 45, 45);
        lv_obj_align(obj_img_2, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_bg_color(obj_img_2, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_color(obj_img_2, lv_palette_main(LV_PALETTE_BLUE), LV_STATE_USER_1);
        lv_obj_set_style_bg_color(obj_img_2, lv_palette_main(LV_PALETTE_GREEN), LV_STATE_USER_2);
        lv_obj_set_style_border_opa(obj_img_2, 0, 0);
        lv_obj_set_style_radius(obj_img_2, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_2, LV_OBJ_FLAG_SCROLLABLE);
        img_2 = lv_img_create(obj_img_2);
        lv_img_set_src(img_2, &img_bt1);
        lv_obj_center(img_2);

        obj_img_3 = lv_obj_create(box_lower_right);
        lv_obj_set_size(obj_img_3, 45, 45);
        lv_obj_align(obj_img_3, LV_ALIGN_TOP_RIGHT, 0, 0);
        lv_obj_set_style_bg_color(obj_img_3, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_color(obj_img_3, lv_palette_main(LV_PALETTE_BLUE), LV_STATE_USER_1);
        lv_obj_set_style_border_opa(obj_img_3, 0, 0);
        lv_obj_set_style_radius(obj_img_3, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_3, LV_OBJ_FLAG_SCROLLABLE);
        img_3 = lv_img_create(obj_img_3);
        lv_img_set_src(img_3, &img_winlock);
        lv_obj_center(img_3);

        obj_img_4 = lv_obj_create(box_lower_right);
        lv_obj_set_size(obj_img_4, 45, 45);
        lv_obj_align(obj_img_4, LV_ALIGN_BOTTOM_LEFT, 0, 0);
        lv_obj_set_style_bg_color(obj_img_4, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_color(obj_img_4, lv_palette_main(LV_PALETTE_BLUE), LV_STATE_USER_1);
        lv_obj_set_style_border_opa(obj_img_4, 0, 0);
        lv_obj_set_style_radius(obj_img_4, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_4, LV_OBJ_FLAG_SCROLLABLE);
        img_4 = lv_img_create(obj_img_4);
        lv_img_set_src(img_4, &img_caplock);
        lv_obj_center(img_4);

        obj_img_5 = lv_obj_create(box_lower_right);
        lv_obj_set_size(obj_img_5, 45, 45);
        lv_obj_align(obj_img_5, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_style_bg_color(obj_img_5, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_color(obj_img_5, lv_palette_main(LV_PALETTE_BLUE), LV_STATE_USER_1);
        lv_obj_set_style_border_opa(obj_img_5, 0, 0);
        lv_obj_set_style_radius(obj_img_5, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_5, LV_OBJ_FLAG_SCROLLABLE);
        img_5 = lv_img_create(obj_img_5);
        lv_img_set_src(img_5, &img_sl);
        lv_obj_center(img_5);

        obj_img_6 = lv_obj_create(box_lower_right);
        lv_obj_set_size(obj_img_6, 45, 45);
        lv_obj_align(obj_img_6, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
        lv_obj_set_style_bg_color(obj_img_6, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_color(obj_img_6, lv_palette_main(LV_PALETTE_BLUE), LV_STATE_USER_1);
        lv_obj_set_style_border_opa(obj_img_6, 0, 0);
        lv_obj_set_style_radius(obj_img_6, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(obj_img_6, LV_OBJ_FLAG_SCROLLABLE);
        img_6 = lv_img_create(obj_img_6);
        lv_img_set_src(img_6, &img_numlock);
        lv_obj_center(img_6);
    }

    void loop()
    {
        lv_arc_set_value(arc_battery, hal.battery_pct);
        lv_label_set_text_fmt(lbl_battery, "%d%%", hal.battery_pct);
        hal.getTime();
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
        if (hal.kb_status.chan_state == 1)
        {
            lv_obj_clear_state(obj_img_2, LV_STATE_USER_2);
            lv_obj_add_state(obj_img_2, LV_STATE_USER_1);
        }
        else if(hal.kb_status.chan_state == 3)
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
        if (hal.kb_status.channel_current == 0)
            lv_img_set_src(img_2, &img_rf);
        else if (hal.kb_status.channel_current == 1)
            lv_img_set_src(img_2, &img_bt1);
        else if (hal.kb_status.channel_current == 2)
            lv_img_set_src(img_2, &img_bt2);
        else if (hal.kb_status.channel_current == 3)
            lv_img_set_src(img_2, &img_usb);
        
        if (hal.kb_status.winlk)
            lv_obj_add_state(obj_img_3, LV_STATE_USER_1);
        else
            lv_obj_clear_state(obj_img_3, LV_STATE_USER_1);

        if (hal.kb_status.capslock)
            lv_obj_add_state(obj_img_4, LV_STATE_USER_1);
        else
            lv_obj_clear_state(obj_img_4, LV_STATE_USER_1);

        if (hal.kb_status.scrolllock)
            lv_obj_add_state(obj_img_5, LV_STATE_USER_1);
        else
            lv_obj_clear_state(obj_img_5, LV_STATE_USER_1);

        if (hal.kb_status.numlock)
            lv_obj_add_state(obj_img_6, LV_STATE_USER_1);
        else
            lv_obj_clear_state(obj_img_6, LV_STATE_USER_1);
    }
} // namespace theme_default

void AppHome::setup()
{
    hal.LOCKLV();
    if (hal.config_theme == 2)
        theme_fox::setup(_appScreen);
    else if (hal.config_theme == 1)
        theme_spartan::setup(_appScreen);
    else
        theme_default::setup(_appScreen);
    hal.UNLOCKLV();
}

void AppHome::loop()
{
    hal.LOCKLV();
    if (hal.config_theme == 2)
        theme_fox::loop();
    else if (hal.config_theme == 1)
        theme_spartan::loop();
    else
        theme_default::loop();
    hal.UNLOCKLV();
    vTaskDelay(560);
}

void AppHome::destroy()
{
}
