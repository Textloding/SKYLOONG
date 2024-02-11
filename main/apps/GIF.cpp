#include "A_Config.h"

// extern "C" const lv_img_dsc_t power_on;
lv_obj_t *gif = NULL;
lv_obj_t *obj_box = NULL;
lv_obj_t *_appScreen_1 = NULL;
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "APP_Def.hpp"
char current_path[1000];
bool file_changed = false;
bool is_folder = false;
void add_btn_file(lv_obj_t *parent, const char *name)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, lv_pct(90), lv_pct(20));
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, name);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(
        btn, [](lv_event_t *e)
        {
            strcpy(current_path, "C:/");
            strcat(current_path, lv_label_get_text(lv_obj_get_child((lv_obj_t *)lv_event_get_target(e), 0)));
            if(strstr(current_path, ".mpeg") != NULL)
            {
                strcpy(current_path, "/littlefs/");
                strcat(current_path, lv_label_get_text(lv_obj_get_child((lv_obj_t *)lv_event_get_target(e), 0)));
            }
            ESP_LOGI("GIF", "current_path: %s", current_path);
            if (strchr(current_path, '.') == NULL)
            {
                GUI::toast(_tr(I18N_ID_OPEN_FOLDER), false);
                strcpy(current_path, "/littlefs/");
                strcat(current_path, lv_label_get_text(lv_obj_get_child((lv_obj_t *)lv_event_get_target(e), 0)));
                is_folder = true;
            }
            else
            {
                is_folder = false;
            }
            file_changed = true;
            lv_obj_fall_down(obj_box, 60, 500, 0);
            lv_obj_del_delayed(obj_box, 500);
            obj_box = NULL; },
        LV_EVENT_CLICKED, NULL);
}
void AppGIF::listFilesRecursively(char *basePath)
{
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir)
        return;
    hal.LOCKLV();

    if (lv_obj_is_valid(obj_box) == false)
        obj_box = NULL;
    if (obj_box != NULL)
    {
        lv_obj_del(obj_box);
        obj_box = NULL;
    }
    obj_box = lv_obj_create(lv_layer_top());
    lv_obj_set_flex_flow(obj_box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj_box, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_size(obj_box, 280, 200);
    lv_obj_align(obj_box, LV_ALIGN_TOP_MID, 0, 300);
    lv_obj_t *label = lv_label_create(obj_box);
    lv_label_set_text(label, _tr(I18N_ID_SELECT_FILE));

    while ((dp = readdir(dir)) != NULL)
    {
        // 构造完整的文件路径
        int len = strlen(dp->d_name);
        if (dp->d_name[0] == '.')
            continue; //.wifi.csv
        add_btn_file(obj_box, dp->d_name);
    }
    lv_obj_move_anim(obj_box, 0x7fff, 60, 500, 0);
    hal.UNLOCKLV();

    // 关闭目录
    closedir(dir);
}
static volatile bool last_setting_mode = false;
volatile bool _vid_stop = false;
volatile bool _vid_playing = false;
static volatile bool _setting_changed = false;
static TaskHandle_t handle_setting;
bool get_vid_stop()
{
    return _vid_stop;
}
static void task_hal_setting_monitor(void *)
{
    while (1)
    {
        if (hal.setting_mode != last_setting_mode)
        {
            last_setting_mode = hal.setting_mode;
            _setting_changed = true;
            if (last_setting_mode == true)
                _vid_stop = true;
        }
        vTaskDelay(500);
    }
}

#include <vector>
std::vector<std::string> gif_list;
int gif_list_idx = 0;
int gif_list_size = 0;
bool show_gif(const char *path)
{
    if (gif)
    {
        lv_obj_fade_out(gif, 1000, 0);
        lv_obj_del_delayed(gif, 1000);
    }
    if (strstr(path, ".jpg") != NULL || strstr(path, ".png") != NULL)
    {
        gif = lv_img_create(_appScreen_1);
        lv_img_set_src(gif, path);
        lv_obj_center(gif);
        lv_obj_fade_in(gif, 1000, 0);
        return true;
    }
    return false;
}
void update_list(const char *basePath)
{
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir)
        return;
    gif_list.clear();
    gif_list_size = 0;
    while ((dp = readdir(dir)) != NULL)
    {
        // 构造完整的文件路径
        int len = strlen(dp->d_name);
        if (dp->d_name[0] == '.')
            continue; //.wifi.csv
        strcpy(path, "C:");
        strcat(path, basePath + 9);
        strcat(path, "/");
        strcat(path, dp->d_name);
        gif_list.push_back(path);
        gif_list_size++;
    }
    if (gif_list_idx >= gif_list_size)
        gif_list_idx = 0;
}
void AppGIF::setup()
{
    _appScreen_1 = _appScreen;
    _vid_stop = false;
    hideStatusBar = true;
    hal.LOCKLV();
    lv_obj_clear_flag(_appScreen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_font(_appScreen, &lv_font_chinese_16, 0);
    lv_obj_set_style_text_font(lv_layer_top(), &lv_font_chinese_16, 0);
    hal.UNLOCKLV();
    xTaskCreatePinnedToCore(task_hal_setting_monitor, "_task_hal_moni", 1024, NULL, 20, &handle_setting, 0);
    if (hal.pref.getString("last_gif", current_path, 1000) != 0)
    {
        file_changed = true;
    }
    else
    {
        listFilesRecursively("/littlefs");
    }
    if (strchr(current_path, '.') == NULL)
    {
        is_folder = true;
        update_list(current_path);
    }
}
uint32_t last_roll_time = 0;
void AppGIF::loop()
{
    if (_vid_stop)
    {
        _vid_stop = false;
        delay(50);
        return;
    }
    if (_vid_playing == false && is_folder)
    {
        if (millis() - last_roll_time > 5000)
        {
            last_roll_time = millis();
            if (gif_list_size > 0)
            {
                gif_list_idx++;
                if (gif_list_idx >= gif_list_size)
                    gif_list_idx = 0;
                hal.LOCKLV();
                show_gif(gif_list.at(gif_list_idx).c_str());
                hal.UNLOCKLV();
            }
        }
    }
    if (_setting_changed)
    {
        _setting_changed = false;
        if (hal.setting_mode)
        {
            listFilesRecursively("/littlefs");
        }
        else if (obj_box)
        {
            hal.LOCKLV();
            lv_obj_fall_down(obj_box, 60, 500, 0);
            lv_obj_del_delayed(obj_box, 500);
            hal.UNLOCKLV();
            obj_box = NULL;
        }
    }
    if (file_changed)
    {
        file_changed = false;
        hal.pref.putString("last_gif", current_path);
        hal.LOCKLV();
        if (is_folder == false)
        {
            if (strstr(current_path, ".mpeg") != NULL)
            {
                if (gif)
                    lv_obj_del(gif);
                gif = NULL;
                hal.UNLOCKLV();
                FILE *f = fopen(current_path, "rb");
                if (f == NULL)
                {
                    GUI::toast(_tr(I18N_ID_FILE_OPEN_FAILED));
                    hal.pref.remove("last_gif");
                    return;
                }
                _vid_playing = true;
                videoPlayer.play(f);
                fclose(f);
                _vid_playing = false;
                lv_obj_invalidate(lv_scr_act());
                hal.LOCKLV();
            }
            else if (strstr(current_path, ".jpg") != NULL || strstr(current_path, ".png") != NULL)
            {
                if (gif)
                    lv_obj_del(gif);
                gif = lv_img_create(_appScreen_1);
                lv_img_set_src(gif, current_path);
                lv_obj_center(gif);
            }
        }
        else
        {
            update_list(current_path);
        }
        last_roll_time = (uint32_t)-80000;
        hal.UNLOCKLV();
    }
}

void AppGIF::destroy()
{
    hal.LOCKLV();
    gif = NULL;
    _appScreen_1 = NULL;
    if (obj_box)
        lv_obj_del(obj_box);
    obj_box = NULL;
    vTaskDelete(handle_setting);
    hal.UNLOCKLV();
}
void AppGIF::stop()
{
    _vid_stop = true;
}
