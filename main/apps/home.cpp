#include "A_Config.h"

static bool f_on = false;
void AppHome::setup()
{
    hal.LOCKLV();
    lv_obj_t *btn = lv_btn_create(_appScreen);
    lv_obj_set_size(btn, LV_SIZE_CONTENT, 50);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Hello world!");
    lv_obj_set_style_text_font(label, &lv_font_chinese_16, 0);
    lv_obj_center(label);
    lv_obj_add_event_cb(
        btn, [](lv_event_t *e)
        { f_on = true; },
        LV_EVENT_CLICKED, NULL);
    hal.UNLOCKLV();
}

void AppHome::loop()
{
    if (f_on)
    {
        f_on = false;
        WiFiMgr.requireWiFi(true);
        WiFi.disconnect(true);
    }
    delay(10);
}

void AppHome::destroy()
{
}
