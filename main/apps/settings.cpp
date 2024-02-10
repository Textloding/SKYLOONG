#include "A_Config.h"

lv_obj_t *create_settings_switch(lv_obj_t *parent, const char *title, const char *str, lv_event_cb_t cb)
{
    lv_obj_t *box = lv_obj_create(parent);
    lv_obj_set_width(box, lv_pct(95));
    lv_obj_set_height(box, LV_SIZE_CONTENT);

    lv_obj_t *lbl_setting_title = lv_label_create(box);
    lv_label_set_text(lbl_setting_title, title);
    lv_obj_align(lbl_setting_title, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_t *lbl_setting_desc = lv_label_create(box);
    lv_label_set_text(lbl_setting_desc, str);
    lv_obj_align_to(lbl_setting_desc, lbl_setting_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_label_set_long_mode(lbl_setting_desc, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lbl_setting_desc, lv_pct(70));
    lv_obj_set_height(lbl_setting_desc, LV_SIZE_CONTENT);
    lv_obj_set_style_text_color(lbl_setting_desc, lv_palette_main(LV_PALETTE_GREY), 0);

    lv_obj_t *sw_btn = lv_switch_create(box);
    lv_obj_align(sw_btn, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event_cb(sw_btn, cb, LV_EVENT_ALL, NULL);
    return sw_btn;
}
lv_obj_t *create_settings_list(lv_obj_t *parent, const char *title, const char *str, const char *dropdown, lv_event_cb_t cb)
{
    lv_obj_t *box = lv_obj_create(parent);
    lv_obj_set_width(box, lv_pct(95));
    lv_obj_set_height(box, LV_SIZE_CONTENT);

    lv_obj_t *lbl_setting_title = lv_label_create(box);
    lv_label_set_text(lbl_setting_title, title);
    lv_obj_align(lbl_setting_title, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_t *lbl_setting_desc = lv_label_create(box);
    lv_label_set_text(lbl_setting_desc, str);
    lv_obj_align_to(lbl_setting_desc, lbl_setting_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_label_set_long_mode(lbl_setting_desc, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lbl_setting_desc, lv_pct(70));
    lv_obj_set_height(lbl_setting_desc, LV_SIZE_CONTENT);
    lv_obj_set_style_text_color(lbl_setting_desc, lv_palette_main(LV_PALETTE_GREY), 0);

    lv_obj_t *lst = lv_dropdown_create(box);
    lv_dropdown_set_options(lst, dropdown);
    lv_obj_set_width(lst, lv_pct(30));
    lv_obj_align(lst, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event_cb(lst, cb, LV_EVENT_ALL, NULL);
    return lst;
}
lv_obj_t *create_settings_button(lv_obj_t *parent, const char *title, const char *str, const char *btn_str, lv_event_cb_t cb)
{
    lv_obj_t *box = lv_obj_create(parent);
    lv_obj_set_width(box, lv_pct(95));
    lv_obj_set_height(box, LV_SIZE_CONTENT);

    lv_obj_t *lbl_setting_title = lv_label_create(box);
    lv_label_set_text(lbl_setting_title, title);
    lv_obj_align(lbl_setting_title, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_t *lbl_setting_desc = lv_label_create(box);
    lv_label_set_text(lbl_setting_desc, str);
    lv_obj_align_to(lbl_setting_desc, lbl_setting_title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_label_set_long_mode(lbl_setting_desc, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lbl_setting_desc, lv_pct(70));
    lv_obj_set_height(lbl_setting_desc, LV_SIZE_CONTENT);
    lv_obj_set_style_text_color(lbl_setting_desc, lv_palette_main(LV_PALETTE_GREY), 0);

    lv_obj_t *btn = lv_btn_create(box);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_ALL, NULL);
    lv_obj_t *lbl_btn = lv_label_create(btn);
    lv_obj_set_style_text_font(lbl_btn, &lv_font_chinese_16, 0);
    lv_label_set_text(lbl_btn, btn_str);
    lv_obj_center(lbl_btn);
    return btn;
}
void create_settings_slider(lv_obj_t *_appScreen, const char *title, lv_event_cb_t cb)
{
    lv_obj_t *box = lv_obj_create(_appScreen);
    lv_obj_set_size(box, lv_pct(95), lv_pct(30));
    lv_obj_center(box);

    lv_obj_t *lbl_title = lv_label_create(box);
    lv_label_set_text(lbl_title, title);
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *slider = lv_slider_create(box);
    lv_obj_set_width(slider, lv_pct(80));
    lv_obj_align(slider, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_slider_set_range(slider, 0, 9);
    lv_slider_set_value(slider, hal._brightness, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, cb, LV_EVENT_ALL, NULL);
}
static bool ntp_req = false;
static int factory_reset_count = 20;
static uint32_t factory_reset_millis_last = 0;
static lv_obj_t *factory_reset_btn = NULL;
void AppSettings::setup()
{
    hal.LOCKLV();
    lv_obj_t *o;
    lv_obj_set_flex_flow(_appScreen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_appScreen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_text_font(_appScreen, &lv_font_chinese_16, 0);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    o = create_settings_switch(_appScreen, _tr(I18N_ID_12HR), _tr(I18N_ID_12HR_DESC), [](lv_event_t *e)
                               {
            if (e->code == LV_EVENT_VALUE_CHANGED)
                hal.config_time_12hr = lv_obj_has_state((lv_obj_t *)lv_event_get_target(e), LV_STATE_CHECKED); });
    if (hal.config_time_12hr)
        lv_obj_add_state(o, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(o, LV_STATE_CHECKED);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    o = create_settings_switch(_appScreen, _tr(I18N_ID_SHOW_BATTERY), _tr(I18N_ID_SHOW_BATTERY_DESC), [](lv_event_t *e)
                               {
            if (e->code == LV_EVENT_VALUE_CHANGED)
                hal.config_show_battery_value = lv_obj_has_state((lv_obj_t *)lv_event_get_target(e), LV_STATE_CHECKED); });
    if (hal.config_show_battery_value)
        lv_obj_add_state(o, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(o, LV_STATE_CHECKED);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    o = create_settings_switch(_appScreen, _tr(I18N_ID_STATUS_BAR_CENTER), _tr(I18N_ID_STATUS_BAR_CENTER_DESC), [](lv_event_t *e)
                               {if (e->code == LV_EVENT_VALUE_CHANGED){hal.config_statusbar_center = lv_obj_has_state((lv_obj_t *)lv_event_get_target(e), LV_STATE_CHECKED); GUI::status_bar_refresh(true);} });
    if (hal.config_statusbar_center)
        lv_obj_add_state(o, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(o, LV_STATE_CHECKED);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    o = create_settings_switch(_appScreen, _tr(I18N_ID_BOOT_ANIMATION), _tr(I18N_ID_BOOT_ANIMATION_DESC), [](lv_event_t *e)
                               {if (e->code == LV_EVENT_VALUE_CHANGED) hal.config_show_boot_animation = lv_obj_has_state((lv_obj_t *)lv_event_get_target(e), LV_STATE_CHECKED); });
    if (hal.config_show_boot_animation)
        lv_obj_add_state(o, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(o, LV_STATE_CHECKED);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    o = create_settings_button(_appScreen, _tr(I18N_ID_START_WEBSERVER), _tr(I18N_ID_START_WEBSERVER_DESC), _tr(I18N_ID_START), [](lv_event_t *e)
                               {
        if (e->code == LV_EVENT_VALUE_CHANGED)
        {
            if(lv_obj_has_state((lv_obj_t *)lv_event_get_target(e), LV_STATE_CHECKED))
            {
                lv_label_set_text(lv_obj_get_child(lv_event_get_target(e), 0), _tr(I18N_ID_STOP));
                hal.send_sysctl(EVENT_SERVERCTL, 1);
            }
            else
            {
                lv_label_set_text(lv_obj_get_child(lv_event_get_target(e), 0), _tr(I18N_ID_START));
                hal.send_sysctl(EVENT_SERVERCTL, 0);
            }
        } });
    lv_obj_add_flag(o, LV_OBJ_FLAG_CHECKABLE);
    if (hal.server_started)
    {
        lv_obj_add_state(o, LV_STATE_CHECKED);
        lv_label_set_text(lv_obj_get_child(o, 0), _tr(I18N_ID_STOP));
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    create_settings_button(_appScreen, _tr(I18N_ID_SYNC_TIME), _tr(I18N_ID_SYNC_TIME_DESC), _tr(I18N_ID_SYNC), [](lv_event_t *e)
                           {
            if (e->code == LV_EVENT_CLICKED)
                ntp_req = true; });
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    create_settings_slider(_appScreen, _tr(I18N_ID_BRIGHTNESS), [](lv_event_t *e)
                           {
            if (e->code == LV_EVENT_VALUE_CHANGED)
                hal.setBrightness(lv_slider_get_value((lv_obj_t*)lv_event_get_target(e))); 
            if(e->code == LV_EVENT_SCREEN_LOADED)
            {
                lv_slider_set_value((lv_obj_t*)lv_event_get_target(e), hal._brightness, LV_ANIM_OFF);
            } });
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    o = create_settings_list(_appScreen, "Language", "修改语言", "中文\nEnglish", [](lv_event_t *e)
                             {
            if (e->code == LV_EVENT_VALUE_CHANGED)
                i18n::setLanguage(lv_dropdown_get_selected((lv_obj_t*)lv_event_get_target(e))); });
    lv_dropdown_set_selected(o, i18n::getLanguage());
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    o = create_settings_list(_appScreen, _tr(I18N_ID_TIMEZONE), _tr(I18N_ID_TIMEZONE_DESC), "UTC-12\nUTC-11\nUTC-10\nUTC-9\nUTC-8\nUTC-7\nUTC-6\nUTC-5\nUTC-4\nUTC-3\nUTC-2\nUTC-1\nUTC+0\nUTC+1\nUTC+2\nUTC+3\nUTC+4\nUTC+5\nUTC+6\nUTC+7\nUTC+8\nUTC+9\nUTC+10\nUTC+11\nUTC+12", [](lv_event_t *e)
                             {

            if (e->code == LV_EVENT_VALUE_CHANGED)
            {
                long t = lv_dropdown_get_selected((lv_obj_t*)lv_event_get_target(e));
                t -= 12;
                t *= 3600;
                i18n::setNTPOffset(t);
            } });
    {
        long t = i18n::getNTPOffset();
        t /= 3600;
        t += 12;
        lv_dropdown_set_selected(o, t);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    factory_reset_btn = create_settings_button(_appScreen, _tr(I18N_ID_FACTORY_RESET), _tr(I18N_ID_FACTORY_RESET_DESC), _tr(I18N_ID_FACTORY_RESET_BTN), [](lv_event_t *e)
                                               {
            if (e->code == LV_EVENT_CLICKED)
            {
                factory_reset_millis_last = millis();
                factory_reset_count--;
                if(factory_reset_count == 0)
                {
                    lv_label_set_text(lv_obj_get_child(factory_reset_btn, 0), _tr(I18N_ID_FACTORY_RESETTING));
                }
                else if(factory_reset_count > 0)
                {
                    lv_label_set_text_fmt(lv_obj_get_child(factory_reset_btn, 0), _tr(I18N_ID_FACTORY_RESETTING_COUNT), factory_reset_count);
                }
            } });
    lv_obj_add_state(factory_reset_btn, LV_STATE_CHECKED);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    lv_obj_set_scroll_snap_y(_appScreen, LV_SCROLL_SNAP_CENTER);
    uint8_t key = LV_KEY_NEXT;
    xQueueSend(hal._queue_kb, &key, 0);
    hal.UNLOCKLV();
    delay(65); // 等待按键事件响应
    hal.LOCKLV();
    lv_obj_scroll_to_view_recursive(lv_group_get_focused(lv_group_get_default()), LV_ANIM_OFF);
    hal.UNLOCKLV();
}

#include "nvs_flash.h"
void AppSettings::loop()
{
    if (ntp_req)
    {
        if (WiFiMgr.requireWiFi())
        {
            if (hal.NTPSync())
                GUI::toast(_tr(I18N_ID_SYNC_SUCCESS));
            else
                GUI::toast(_tr(I18N_ID_SYNC_FAILED));
        }
        ntp_req = false;
    }
    if (factory_reset_count != 20)
    {
        if (millis() - factory_reset_millis_last > 1000)
        {
            factory_reset_millis_last = 0;
            factory_reset_count = 20;
            lv_label_set_text(lv_obj_get_child(factory_reset_btn, 0), _tr(I18N_ID_FACTORY_RESET_BTN));
        }
    }
    if (factory_reset_count <= 0)
    {
        hal.LOCKLV();
        nvs_flash_erase();
        LittleFS.format();
        ESP.restart();
    }
}

void AppSettings::destroy()
{
    if (hal.server_started == false)
        WiFiMgr.disconnect();
    hal.pref.putUInt("bright", hal._brightness);
    hal.pref.putBool("12hr", hal.config_time_12hr);
    hal.pref.putBool("s_b_v", hal.config_show_battery_value);
    hal.pref.putBool("s_c", hal.config_statusbar_center);
    hal.pref.putBool("b_a", hal.config_show_boot_animation);
    hal.pref.putUInt("lang", i18n::getLanguage());
    hal.pref.putInt("ntp", i18n::getNTPOffset());
}
