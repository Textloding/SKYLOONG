#include "A_Config.h"
#include <stdio.h>
#include <stdlib.h>

extern "C" const lv_img_dsc_t img_APS;

lv_obj_t *chart_aps = NULL;
lv_chart_series_t *ser1 = NULL;
lv_obj_t *lbl_aps = NULL;
lv_obj_t *lbl_apm = NULL;
lv_obj_t *lbl_keyN = NULL;
#include <math.h>
int key_total = 0;
int time_cnt = 0;
void set_aps(int aps)
{
    key_total += aps;
    time_cnt++;
    if (key_total >= 100000)
    {
        key_total = 0;
        time_cnt = 1;
    }
    lv_label_set_text_fmt(lbl_aps, "%d", aps);
    lv_label_set_text_fmt(lbl_apm, "%d", key_total * 60 / time_cnt);
    lv_label_set_text_fmt(lbl_keyN, "%d", key_total);
    if (time_cnt < 30)
        lv_chart_set_update_mode(chart_aps, LV_CHART_UPDATE_MODE_CIRCULAR);
    else
        lv_chart_set_update_mode(chart_aps, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_next_value(chart_aps, ser1, (int)(log10(aps) * 100));
}
void AppAPS::setup()
{
    time_cnt = 0;
    key_total = 0;
    hal.LOCKLV();
    lv_obj_set_style_bg_img_src(_appScreen, &img_APS, 0);
    lbl_aps = lv_label_create(_appScreen);
    lv_label_set_text(lbl_aps, "0");
    lv_obj_set_style_text_align(lbl_aps, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(lbl_aps, 251, 80);
    lv_obj_set_size(lbl_aps, 50, 22);
    lv_obj_set_style_text_color(lbl_aps, lv_color_black(), 0);

    lbl_apm = lv_label_create(_appScreen);
    lv_label_set_text(lbl_apm, "0");
    lv_obj_set_style_text_align(lbl_apm, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(lbl_apm, 251, 132);
    lv_obj_set_size(lbl_apm, 50, 22);
    lv_obj_set_style_text_color(lbl_apm, lv_color_black(), 0);

    lbl_keyN = lv_label_create(_appScreen);
    lv_label_set_text(lbl_keyN, "0");
    lv_obj_set_style_text_align(lbl_keyN, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(lbl_keyN, 251, 188);
    lv_obj_set_size(lbl_keyN, 50, 22);
    lv_obj_set_style_text_color(lbl_keyN, lv_color_black(), 0);

    chart_aps = lv_chart_create(_appScreen);
    lv_obj_set_style_bg_opa(chart_aps, LV_OPA_0, 0);
    lv_obj_set_style_border_opa(chart_aps, LV_OPA_0, 0);
    lv_obj_set_style_line_opa(chart_aps, LV_OPA_0, 0);
    lv_obj_set_pos(chart_aps, 25, 54);
    lv_obj_set_size(chart_aps, 206, 162);
    lv_chart_set_type(chart_aps, LV_CHART_TYPE_LINE);
    ser1 = lv_chart_add_series(chart_aps, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_point_count(chart_aps, 30);
    lv_obj_set_style_pad_all(chart_aps, 0, 0);
    lv_chart_set_range(chart_aps, LV_CHART_AXIS_PRIMARY_Y, 0, 247);
    hal.UNLOCKLV();
}

void AppAPS::loop()
{
    if (hal.APMChanged)
    {
        hal.APMChanged = false;
        if (hal.APM != 0)
            set_aps(hal.APM);
    }
}

void AppAPS::destroy()
{
}
