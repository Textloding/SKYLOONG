#include "A_Config.h"
lv_obj_t *gif = NULL;
lv_obj_t *_appScreen_1 = NULL;
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "APP_Def.hpp"
uint32_t last_roll_time = 0;
volatile bool _vid_stop = false;
bool get_vid_stop()
{
    return _vid_stop;
}

#include <vector>
std::vector<std::string> gif_list;
int gif_list_idx = 0;
int gif_list_size = 0;
bool show_gif(const char *path)
{
    if (strstr(path, ".mpeg") != NULL)
    {
        if (gif)
            lv_obj_del(gif);
        gif = NULL;
        char path_new[256];
        strcpy(path_new, "/littlefs");
        strncpy(path_new + 9, path + 2, 240);
        ESP_LOGI("GIF", "Playing video %s", path_new);
        FILE *f = fopen(path_new, "rb");
        if (f == NULL)
        {
            hal.pref.remove("last_gif");
            return false;
        }
        hal.UNLOCKLV();
        videoPlayer.play(f);
        hal.LOCKLV();
        fclose(f);
        lv_obj_invalidate(lv_scr_act());
        return true;
    }
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
    hal.LOCKLV();
    lv_obj_clear_flag(_appScreen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_font(_appScreen, &lv_font_chinese_16, 0);
    lv_obj_set_style_text_font(lv_layer_top(), &lv_font_chinese_16, 0);
    hal.UNLOCKLV();
    update_list("/littlefs");
    last_roll_time = -10000;
}
void AppGIF::loop()
{
    if (_vid_stop)
    {
        _vid_stop = false;
        delay(50);
        return;
    }
    if (millis() - last_roll_time > hal.config_time_roll)
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

void AppGIF::destroy()
{
    hal.LOCKLV();
    gif = NULL;
    _appScreen_1 = NULL;
    hal.UNLOCKLV();
}
void AppGIF::stop()
{
    _vid_stop = true;
}
