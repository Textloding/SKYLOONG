#include "A_Config.h"
lv_obj_t *_appScreen_gif = NULL;
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "APP_Def.hpp"
uint32_t last_gif_roll_time = 0;
volatile bool gif_vid_stop = false;
bool get_gif_vid_stop()
{
    return gif_vid_stop;
}

#include <vector>
std::vector<std::string> gif_list;
int gif_list_idx = 0;
int gif_list_size = 0;
bool show_gif(const char *path)
{
    if (strstr(path, ".mpeg") != NULL)
    {
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
        if (gif_list_size > 1)
            videoPlayer.video_loop = false;
        else
            videoPlayer.video_loop = true;
        videoPlayer.play(f);
        hal.LOCKLV();
        fclose(f);
        lv_obj_invalidate(lv_scr_act());
        last_gif_roll_time = 0;
        return true;
    }
    return false;
}
void update_gif_list(const char *basePath)
{
    char path[100];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir)
        return;
    gif_list.clear();
    gif_list_size = 0;
    while ((dp = readdir(dir)) != NULL)
    {
        if (dp->d_name[0] == '.')
            continue;
        if (strstr(dp->d_name, ".mpeg") == NULL)
            continue;
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
    _appScreen_gif = _appScreen;
    gif_vid_stop = false;
    hal.LOCKLV();
    lv_obj_clear_flag(_appScreen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_font(_appScreen, &lv_font_chinese_16, 0);
    lv_obj_set_style_text_font(lv_layer_top(), &lv_font_chinese_16, 0);
    hal.UNLOCKLV();
    update_gif_list("/littlefs");
    last_gif_roll_time = 0;
    hal.gif_update = true;
}

void AppGIF::loop()
{
    if (!hal.gif_enable) {
        xSemaphoreGive(appManagerLite._binary_switchApp);
        return;
    }
    if (hal.gif_update == false) {
        hal.gif_update = true;
        hal.send_sysctl(EVENT_GIF_REFRESH);
        return;
    }

    if (gif_list.size() == 0)
    {
        xSemaphoreGive(appManagerLite._binary_switchApp);
        return;
    }
    if (gif_vid_stop)
    {
        gif_vid_stop = false;
        delay(50);
        return;
    }
    if (millis() - last_gif_roll_time > hal.config_time_roll || last_gif_roll_time == 0)
    {
        ESP_LOGI("GIF", "Rolling");
        last_gif_roll_time = millis();
        if (gif_list_size > 0)
        {
            gif_list_idx++;
            if (gif_list_idx >= gif_list_size)
                gif_list_idx = 0;
            hal.LOCKLV();
            if (show_gif(gif_list.at(gif_list_idx).c_str()) == false)
            {
                hal.UNLOCKLV();
                last_gif_roll_time = 0;
                return;
            }
            hal.UNLOCKLV();
        }
    }
}

void AppGIF::destroy()
{
    hal.LOCKLV();
    _appScreen_gif = NULL;
    hal.UNLOCKLV();
}
void AppGIF::stop()
{
    gif_vid_stop = true;
}
