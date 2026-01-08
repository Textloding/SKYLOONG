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
    lv_obj_set_width(lbl_setting_desc, lv_pct(60));
    lv_obj_set_height(lbl_setting_desc, LV_SIZE_CONTENT);
    lv_obj_set_style_text_color(lbl_setting_desc, lv_palette_main(LV_PALETTE_GREY), 0);

    lv_obj_t *lst = lv_dropdown_create(box);
    lv_dropdown_set_options(lst, dropdown);
    lv_obj_set_width(lst, lv_pct(40));
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

void create_settings_slider(lv_obj_t *_appScreen, const char *title, int32_t value, lv_event_cb_t cb)
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
    lv_slider_set_value(slider, value, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, cb, LV_EVENT_ALL, NULL);
}

static bool server_req = false;
static bool ntp_req = false;
static bool rechoose_wifi = false;
static bool update_req = false;
static int factory_reset_count = 20;
static bool reboot_needed = false;
static uint32_t factory_reset_millis_last = 0;
static lv_obj_t *ota_update_btn = NULL;
static lv_obj_t *factory_reset_btn = NULL;
static lv_obj_t *btn_server = NULL;
lv_obj_t *exit_btn = NULL;

void AppSettings::setup()
{
    hal.LOCKLV();
    lv_obj_t *o;
    lv_group_set_wrap(lv_group_get_default(), false);
    lv_obj_set_flex_flow(_appScreen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_appScreen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_text_font(_appScreen, &lv_font_chinese_16, 0);

    o = create_settings_button(_appScreen, _tr(I18N_ID_RECHOOSE_WIFI), _tr(I18N_ID_RECHOOSE_WIFI_DESC), _tr(I18N_ID_RECHOOSE), [](lv_event_t *e)
                               {
        if (e->code == LV_EVENT_CLICKED)
            rechoose_wifi = true; 
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF); });

    o = create_settings_switch(_appScreen, _tr(I18N_ID_BOOT_ANIMATION), _tr(I18N_ID_BOOT_ANIMATION_DESC), [](lv_event_t *e)
                               {
        if (e->code == LV_EVENT_VALUE_CHANGED)
            hal.config_bootanimation = lv_obj_has_state((lv_obj_t *)lv_event_get_target(e), LV_STATE_CHECKED);
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF); });
    if (hal.config_bootanimation)
        lv_obj_add_state(o, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(o, LV_STATE_CHECKED);

    o = create_settings_switch(_appScreen, _tr(I18N_ID_12HR), _tr(I18N_ID_12HR_DESC), [](lv_event_t *e)
                               {
        if (e->code == LV_EVENT_VALUE_CHANGED)
            hal.config_time_12hr = lv_obj_has_state((lv_obj_t *)lv_event_get_target(e), LV_STATE_CHECKED);
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF); });
    if (hal.config_time_12hr)
        lv_obj_add_state(o, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(o, LV_STATE_CHECKED);
    lv_obj_clear_state(o, LV_STATE_CHECKED);

    btn_server = create_settings_button(_appScreen, _tr(I18N_ID_START_WEBSERVER), _tr(I18N_ID_START_WEBSERVER_DESC), _tr(I18N_ID_START), [](lv_event_t *e)
    {
        if (e->code == LV_EVENT_VALUE_CHANGED)
        {
            if(lv_obj_has_state((lv_obj_t *)lv_event_get_target(e), LV_STATE_CHECKED))
            {
                lv_obj_add_state(btn_server, LV_STATE_DISABLED);
                server_req = true;
            }
            else
            {
                lv_label_set_text(lv_obj_get_child(lv_event_get_target(e), 0), _tr(I18N_ID_START));
                hal.send_sysctl(EVENT_SERVERCTL, 0);
            }
        }
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF); });
    lv_obj_add_flag(btn_server, LV_OBJ_FLAG_CHECKABLE);
    if (hal.server_started)
    {
        lv_obj_add_state(btn_server, LV_STATE_CHECKED);
    }

    create_settings_button(_appScreen, _tr(I18N_ID_SYNC_TIME), _tr(I18N_ID_SYNC_TIME_DESC), _tr(I18N_ID_SYNC), [](lv_event_t *e)
                           {
        if (e->code == LV_EVENT_CLICKED)
            ntp_req = true;
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF); });

    create_settings_slider(_appScreen, _tr(I18N_ID_BRIGHTNESS), hal._brightness, [](lv_event_t *e) {
        if (e->code == LV_EVENT_VALUE_CHANGED)
            hal.setBrightness(lv_slider_get_value((lv_obj_t*)lv_event_get_target(e))); 
        if(e->code == LV_EVENT_SCREEN_LOADED)
        {
            lv_slider_set_value((lv_obj_t*)lv_event_get_target(e), hal._brightness, LV_ANIM_OFF);
        }
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF);
    });

    create_settings_slider(_appScreen, _tr(I18N_ID_VOLUME), hal._volume, [](lv_event_t *e) {
        if (e->code == LV_EVENT_VALUE_CHANGED)
            hal.setVolume(lv_slider_get_value((lv_obj_t*)lv_event_get_target(e))); 
        if(e->code == LV_EVENT_SCREEN_LOADED)
        {
            lv_slider_set_value((lv_obj_t*)lv_event_get_target(e), hal._volume, LV_ANIM_OFF);
        }
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF);
    });

    o = create_settings_list(_appScreen, _tr(I18N_ID_THEME), _tr(I18N_ID_CHANGE_THEME), _tr(I18N_ID_THEME_LIST), [](lv_event_t *e) {
        if (e->code == LV_EVENT_VALUE_CHANGED)
        {
            hal.config_theme = lv_dropdown_get_selected((lv_obj_t*)lv_event_get_target(e));
        }
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF);
    });
    lv_dropdown_set_selected(o, hal.config_theme);

    o = create_settings_list(_appScreen, _tr(I18N_ID_KEYTONE), _tr(I18N_ID_CHANGE_KEYTONE), _tr(I18N_ID_KEYTONE_LIST), [](lv_event_t *e) {
        if (e->code == LV_EVENT_VALUE_CHANGED)
        {
            hal.config_keytone = lv_dropdown_get_selected((lv_obj_t*)lv_event_get_target(e));
        }
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF);
    });
    lv_dropdown_set_selected(o, hal.config_keytone);

    o = create_settings_list(_appScreen, _tr(I18N_ID_ROLL_TIME), _tr(I18N_ID_ROLL_TIME_DESC), "2s\n3s\n4s\n5s\n6s\n7s\n8s\n9s\n10s\n11s\n12s\n13s\n14s\n15s", [](lv_event_t *e)
                             {
        if (e->code == LV_EVENT_VALUE_CHANGED)
        {
            hal.config_time_roll = (lv_dropdown_get_selected((lv_obj_t*)lv_event_get_target(e)) + 2) * 1000;
        }
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF); });
    lv_dropdown_set_selected(o, hal.config_time_roll / 1000 - 2);

    o = create_settings_list(_appScreen, "Language", "修改语言", "中文\nEnglish", [](lv_event_t *e)
                             {
        if (e->code == LV_EVENT_VALUE_CHANGED)
            i18n::setLanguage(lv_dropdown_get_selected((lv_obj_t*)lv_event_get_target(e)));
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF); });
    lv_dropdown_set_selected(o, i18n::getLanguage());

    o = create_settings_list(_appScreen, _tr(I18N_ID_TIMEZONE), _tr(I18N_ID_TIMEZONE_DESC), "UTC-12\nUTC-11\nUTC-10\nUTC-9\nUTC-8\nUTC-7\nUTC-6\nUTC-5\nUTC-4\nUTC-3\nUTC-2\nUTC-1\nUTC+0\nUTC+1\nUTC+2\nUTC+3\nUTC+4\nUTC+5\nUTC+6\nUTC+7\nUTC+8\nUTC+9\nUTC+10\nUTC+11\nUTC+12", [](lv_event_t *e)
                             {

        if (e->code == LV_EVENT_VALUE_CHANGED)
        {
            long t = lv_dropdown_get_selected((lv_obj_t*)lv_event_get_target(e));
            t -= 12;
            t *= 3600;
            i18n::setNTPOffset(t);
            ntp_req = true;
        }
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF); });
    {
        long t = i18n::getNTPOffset();
        t /= 3600;
        t += 12;
        lv_dropdown_set_selected(o, t);
    }

    o = create_settings_switch(_appScreen, _tr(I18N_ID_ENABLE_APS_APP), _tr(I18N_ID_ENABLE_APS_APP_DESC), [](lv_event_t *e)
                               {
        if (e->code == LV_EVENT_VALUE_CHANGED){
            reboot_needed = true;
            hal.pref.putBool("aps_enable", lv_obj_has_state((lv_obj_t *)lv_event_get_target(e), LV_STATE_CHECKED));
        }
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF); });
    if (hal.aps_enable)
        lv_obj_add_state(o, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(o, LV_STATE_CHECKED);

    o = create_settings_switch(_appScreen, _tr(I18N_ID_ENABLE_GIF_APP), _tr(I18N_ID_ENABLE_GIF_APP_DESC), [](lv_event_t *e)
                               {
        if (e->code == LV_EVENT_VALUE_CHANGED){
            reboot_needed = true;
            hal.pref.putBool("gif_enable", lv_obj_has_state((lv_obj_t *)lv_event_get_target(e), LV_STATE_CHECKED));
        }
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF); });
    if (hal.pref.getBool("gif_enable", true))
        lv_obj_add_state(o, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(o, LV_STATE_CHECKED);

    if (i18n::getLanguage() == 0) {
        o = create_settings_switch(_appScreen, _tr(I18N_ID_ENABLE_WEATHER_APP), _tr(I18N_ID_ENABLE_WEATHER_APP_DESC), [](lv_event_t *e)
                                {
            if (e->code == LV_EVENT_VALUE_CHANGED){
                reboot_needed = true;
                hal.pref.putBool("weather_enable", lv_obj_has_state((lv_obj_t *)lv_event_get_target(e), LV_STATE_CHECKED));
            }
            if (e->code == LV_EVENT_FOCUSED)
                lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF); });
        if (hal.pref.getBool("weather_enable", true))
            lv_obj_add_state(o, LV_STATE_CHECKED);
        else
            lv_obj_clear_state(o, LV_STATE_CHECKED);
    }

    o = create_settings_switch(_appScreen, _tr(I18N_ID_ENABLE_PERF_APP), _tr(I18N_ID_ENABLE_PERF_APP_DESC), [](lv_event_t *e)
                               {
        if (e->code == LV_EVENT_VALUE_CHANGED){
            reboot_needed = true;
            hal.pref.putBool("sysinfo_enable", lv_obj_has_state((lv_obj_t *)lv_event_get_target(e), LV_STATE_CHECKED));
        }
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF); });
    if (hal.pref.getBool("sysinfo_enable", true))
        lv_obj_add_state(o, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(o, LV_STATE_CHECKED);

    ota_update_btn = create_settings_button(_appScreen, _tr(I18N_ID_FIRMWARE_VERSION), FIRMWARE_VERSION, _tr(I18N_ID_CHECK_UPDATE), [](lv_event_t *e)
                                            {
        if (e->code == LV_EVENT_CLICKED)
        {
            update_req = true;
        }
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF); });

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
        }
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF); });
    lv_obj_add_state(factory_reset_btn, LV_STATE_CHECKED);

    exit_btn = create_settings_button(_appScreen, _tr(I18N_ID_EXIT_TITLE), _tr(I18N_ID_EXIT_DESC), _tr(I18N_ID_EXIT), [](lv_event_t *e)
                                      {
            if (e->code == LV_EVENT_CLICKED)
            {
                hal.forceExitSettings();
            }
        if (e->code == LV_EVENT_FOCUSED)
            lv_obj_scroll_to_view(e->target->parent, LV_ANIM_OFF); });
    lv_obj_add_state(exit_btn, LV_STATE_CHECKED);

    hal.UNLOCKLV();
}

#include "nvs_flash.h"
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "cJSON.h"
#define REMOTE_UPDATE_URL "http://cloudmouse.oss-cn-beijing.aliyuncs.com/FWFile/screen_4/update.json"
String update_bin_url = "";
int update_version_int = 0;
String update_version_str = "";
uint32_t last_millis = 0;
HTTPClient http;
lv_obj_t *scr_update_ui;
lv_obj_t *lbl_update_info;
lv_obj_t *progress_update;
lv_obj_t *lbl_update_progress;

void AppSettings::loop()
{
    if (millis() - last_millis > 300)
    {
        last_millis = millis();
        if (hal.server_started)
        {
            if (WiFi.getMode() == WIFI_AP)
                lv_label_set_text(lv_obj_get_child(btn_server, 0), "192.168.4.1");
            else if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED)
                lv_label_set_text(lv_obj_get_child(btn_server, 0), WiFi.localIP().toString().c_str());
            else
                lv_label_set_text(lv_obj_get_child(btn_server, 0), _tr(I18N_ID_STOP));
        }
        else
        {
            lv_label_set_text(lv_obj_get_child(btn_server, 0), _tr(I18N_ID_START));
        }
    }
    if (rechoose_wifi)
    {
        WiFi.disconnect(true);
        WiFiMgr.requireWiFi(true);
        rechoose_wifi = false;
    }
    if (server_req)
    {
        WiFiMgr.requireWiFi();
        hal.send_sysctl(EVENT_SERVERCTL, 1);
        lv_obj_clear_state(btn_server, LV_STATE_DISABLED);
        server_req = false;
    }
    if (ntp_req)
    {
        if (WiFiMgr.requireWiFi())
        {
            if (hal.NTPSync()) {
                GUI::toast(_tr(I18N_ID_SYNC_SUCCESS));
                hal.time_sync = true;
            } else {
                GUI::toast(_tr(I18N_ID_SYNC_FAILED));
            }
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
        hal.forceExitSettings();
        hal.LOCKLV();
        hal.pref.end();
        LittleFS.end();
        nvs_flash_erase();
        LittleFS.format();
        nvs_flash_init();
        if (LittleFS.begin(false)) {
            hal.pref.begin("settings", false);
            hal.pref.putUInt("lang", i18n::getLanguage());
        }
        delay(1000);
        ESP.restart();
    }
    if (update_req)
    {
        if (WiFiMgr.requireWiFi())
        {
            WiFiClient client;
            if (lv_obj_has_state(ota_update_btn, LV_STATE_CHECKED) == true && update_bin_url.length() > 4)
            {
                scr_update_ui = lv_obj_create(NULL);
                lv_obj_set_style_text_font(scr_update_ui, &lv_font_chinese_16, 0);
                lbl_update_info = lv_label_create(scr_update_ui);
                lv_label_set_text(lbl_update_info, _tr(I18N_ID_UPDATING));
                lv_obj_set_width(lbl_update_info, lv_pct(80));
                lv_obj_align(lbl_update_info, LV_ALIGN_TOP_MID, 0, 20);
                lv_obj_set_style_text_align(lbl_update_info, LV_TEXT_ALIGN_CENTER, 0);
                lv_label_set_long_mode(lbl_update_info, LV_LABEL_LONG_WRAP);
                progress_update = lv_bar_create(scr_update_ui);
                lv_obj_set_width(progress_update, lv_pct(80));
                lv_obj_align(progress_update, LV_ALIGN_BOTTOM_MID, 0, -60);
                lv_obj_set_style_opa(progress_update, LV_OPA_0, LV_PART_KNOB);
                lv_bar_set_value(progress_update, 0, LV_ANIM_OFF);
                lv_bar_set_range(progress_update, 0, 100);
                lbl_update_progress = lv_label_create(scr_update_ui);
                lv_label_set_text(lbl_update_progress, _tr(I18N_ID_PREPARE_UPDATE));
                lv_obj_align(lbl_update_progress, LV_ALIGN_BOTTOM_MID, 0, -30);
                lv_obj_set_style_text_align(lbl_update_progress, LV_TEXT_ALIGN_CENTER, 0);
                lv_scr_load_anim(scr_update_ui, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
                httpUpdate.onStart([]
                                   { lv_label_set_text_fmt(lbl_update_progress, _tr(I18N_ID_UPDATE_INITIALIZED)); });
                httpUpdate.onEnd([]
                                 { lv_label_set_text_fmt(lbl_update_progress, _tr(I18N_ID_UPDATE_SUCCESS)); vTaskDelay(2000); });
                httpUpdate.onProgress([](int cur, int total)
                                      {
                    lv_bar_set_value(progress_update, (int)(cur * 100 / total), LV_ANIM_OFF);
                    lv_label_set_text_fmt(lbl_update_progress, "%d%%", (int)(cur * 100 / total)); });
                httpUpdate.onError([](int err)
                                   {
                    GUI::toast(_tr(I18N_ID_UPDATE_FAILED));
                    ESP_LOGE("OTA", "HTTP update failed with error %d", err);
                    vTaskDelay(5000);
                    ESP.restart(); });
                t_httpUpdate_return ret = httpUpdate.update(client, update_bin_url);
                switch (ret)
                {
                case HTTP_UPDATE_FAILED:
                    ESP_LOGE("OTA", "HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
                    break;

                case HTTP_UPDATE_NO_UPDATES:
                    ESP_LOGE("OTA", "HTTP_UPDATE_NO_UPDATES");
                    break;

                case HTTP_UPDATE_OK:
                    ESP_LOGE("OTA", "HTTP_UPDATE_OK");
                    break;
                }
                vTaskDelay(5000);
                ESP.restart();
            }
            else
            {
                http.begin(REMOTE_UPDATE_URL);
                int httpCode = http.GET();
                if (httpCode == 200)
                {
                    String payload = http.getString();
                    cJSON *root = cJSON_Parse(payload.c_str());
                    cJSON *version = cJSON_GetObjectItem(root, "version");
                    cJSON *version_int = cJSON_GetObjectItem(root, "version_int");
                    cJSON *url = cJSON_GetObjectItem(root, "url");
                    update_version_int = version_int->valueint;
                    update_version_str = version->valuestring;
                    update_bin_url = url->valuestring;
                    if (update_bin_url.length() > 0)
                    {
                        if (FIRMWARE_VERSION_INT < update_version_int)
                        {
                            ESP_LOGI("OTA", "New version %d", update_version_int);
                            ESP_LOGI("OTA", "%s", update_version_str.c_str());
                            ESP_LOGI("OTA", "Start updating firmware from %s", update_bin_url.c_str());
                            GUI::toast(_tr(I18N_ID_NEW_VERSION));
                            lv_obj_add_state(ota_update_btn, LV_STATE_CHECKED);
                            lv_label_set_text(lv_obj_get_child(ota_update_btn, 0), _tr(I18N_ID_UPDATE_IMMEDIATELY));
                        }
                        else
                        {
                            GUI::toast(_tr(I18N_ID_NO_UPDATE));
                        }
                    }
                    else
                    {
                        GUI::toast(_tr(I18N_ID_INTERNAL_ERR));
                    }
                }
                else
                {
                    GUI::toast(_tr(I18N_ID_CANNOT_CONNECT_TO_SERVER));
                }
                http.end();
            }
        }
        update_req = false;
    }
}

void AppSettings::destroy()
{
    if (hal.server_started)
    {
        GUI::toast(_tr(I18N_ID_WEBSERVER_RUNNING_BACKGROUND));
    }
    else
    {
        WiFi.disconnect(true);
    }
    hal.pref.putUInt("bright", hal._brightness);
    hal.pref.putUInt("volume", hal._volume);
    hal.pref.putBool("12hr", hal.config_time_12hr);
    hal.pref.putBool("s_b_a", hal.config_bootanimation);
    hal.pref.putInt("t_r", hal.config_time_roll);
    hal.pref.putInt("theme", hal.config_theme);
    hal.pref.putInt("keytone", hal.config_keytone);
    hal.pref.putUInt("lang", i18n::getLanguage());
    hal.pref.putInt("ntp", i18n::getNTPOffset());
    if (reboot_needed)
    {
        hal.forceExitSettings();
        delay(2000);
        ESP.restart();
    }
}
