#include "A_Config.h"
lv_obj_t *jpg = NULL;
lv_obj_t *_appScreen_jpg = NULL;
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "APP_Def.hpp"
uint32_t last_jpg_roll_time = 0;

#include <vector>
std::vector<std::string> jpg_list;
int jpg_list_idx = 0;
int jpg_list_size = 0;
char show_jpg_file[32];
bool show_jpg(const char *path)
{
    if (jpg)
    {
        lv_img_cache_invalidate_src(lv_img_get_src(jpg));
        lv_obj_del(jpg);
        jpg = NULL;
    }
    if (strstr(path, ".jpg") != NULL)
    {
        ESP_LOGI("JPG", "Playing image %s", path);
        jpg = lv_img_create(_appScreen_jpg);
        lv_img_set_src(jpg, path);
        lv_obj_center(jpg);
        lv_obj_fade_in(jpg, 1000, 0);
        strcpy(show_jpg_file, path);
        return true;
    }
    return false;
}
void update_jpg_list(const char *basePath)
{
    char path[100];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir)
        return;
    jpg_list.clear();
    jpg_list_size = 0;
    while ((dp = readdir(dir)) != NULL)
    {
        if (dp->d_name[0] == '.')
            continue;
        if (strstr(dp->d_name, ".jpg") == NULL)
            continue;
        strcpy(path, "C:");
        strcat(path, basePath + 9);
        strcat(path, "/");
        strcat(path, dp->d_name);
        jpg_list.push_back(path);
        jpg_list_size++;
    }
    if (jpg_list_idx >= jpg_list_size)
        jpg_list_idx = 0;
}
void AppJPG::setup()
{
    _appScreen_jpg = _appScreen;
    hal.LOCKLV();
    lv_obj_clear_flag(_appScreen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_font(_appScreen, &lv_font_chinese_16, 0);
    lv_obj_set_style_text_font(lv_layer_top(), &lv_font_chinese_16, 0);
    hal.UNLOCKLV();
    update_jpg_list("/littlefs");
    last_jpg_roll_time = 0;
    hal.jpg_update = true;
}

void AppJPG::loop()
{
    if (!hal.jpg_enable) {
        xSemaphoreGive(appManagerLite._binary_switchApp);
        return;
    }
    if (hal.jpg_update == false) {
        hal.jpg_update = true;
        hal.send_sysctl(EVENT_JPG_REFRESH);
        return;
    }

    if (jpg_list.size() == 0)
    {
        xSemaphoreGive(appManagerLite._binary_switchApp);
        return;
    }
    if (strcmp(hal.config_jpg_mode, "fixed") == 0) {
        char path[32];
        strcpy(path, "C:/");
        strcat(path, hal.config_jpg_file);
        if (strcmp(path,show_jpg_file) != 0) {
        	hal.LOCKLV();
            if (show_jpg(path) == false) {
				hal.UNLOCKLV();
                xSemaphoreGive(appManagerLite._binary_switchApp);
				return;
            }
			hal.UNLOCKLV();
        }
    } else {
        if (millis() - last_jpg_roll_time > hal.config_time_roll || last_jpg_roll_time == 0)
        {
            ESP_LOGI("JPG", "Rolling");
            last_jpg_roll_time = millis();
            if (jpg_list_size > 0)
            {
                jpg_list_idx++;
                if (jpg_list_idx >= jpg_list_size)
                    jpg_list_idx = 0;
                hal.LOCKLV();
                if (show_jpg(jpg_list.at(jpg_list_idx).c_str()) == false)
                {
                    hal.UNLOCKLV();
                    last_jpg_roll_time = 0;
                    return;
                }
                hal.UNLOCKLV();
            }
        }
    }
}

void AppJPG::destroy()
{
    hal.LOCKLV();
    if (jpg) {
        lv_img_cache_invalidate_src(lv_img_get_src(jpg));
        lv_obj_del(jpg);
        jpg = NULL;
    }
    _appScreen_jpg = NULL;
    hal.UNLOCKLV();
}
void AppJPG::stop()
{
}
