#pragma once

#define GUI_ANIM_FLOATING_RANGE 8
#define GUI_ANIM_FLOATING_TIME 1000
/**
 * @brief 向下弹出动画
 * @param obj lvgl对象指针
 * @param distance 动画移动距离
 * @param time 动画持续时长
 * @param delay 动画开始前延时
 */
void lv_obj_push_down(lv_obj_t *obj, uint16_t distance = 24,
                      uint16_t time = 300, uint16_t waitBeforeStart = 0);

/**
 * @brief 向上弹出动画
 * @param obj lvgl对象指针
 * @param distance 动画移动距离
 * @param time 动画持续时长
 * @param delay 动画开始前延时
 */
void lv_obj_pop_up(lv_obj_t *obj, uint16_t distance = 30,
                   uint16_t time = 300, uint16_t waitBeforeStart = 0);

void lv_obj_fall_down(lv_obj_t *obj, uint16_t distance = 30,
                      uint16_t time = 300, uint16_t waitBeforeStart = 0);

/**
 * @brief 浮动动画
 * @param obj lvgl对象指针
 * @param waitBeforeStart 动画开始前延时
 */
void lv_obj_floating_add(lv_obj_t *obj, uint16_t waitBeforeStart = 0);

/**
 * @brief 移动到指定位置，渐慢动画
 * @param obj lvgl对象指针
 * @param x 目标位置x坐标
 * @param y 目标位置y坐标
 * @param time 动画持续时长
 * @param delay 动画开始前延时
 */
void lv_obj_move_anim(lv_obj_t *obj, int16_t x, int16_t y,
                      uint16_t time = 500, uint16_t waitBeforeStart = 0);
namespace GUI
{
    void toast(const char *str, bool lock = true);
    void status_bar_refresh(bool force = false);
    lv_obj_t *status_bar_add(const char *symbol, int delay_ms);
    void status_bar_remove(lv_obj_t *obj);
    void status_bar_status_set(lv_obj_t *obj, lv_state_t status);
    lv_obj_t *status_bar_insert(const char *symbol, int index);
    lv_obj_t *status_bar_replace(const char *symbol, lv_obj_t *&last);
    void status_bar_init();
    void status_bar_show(bool show);
    bool status_bar_show_get();
}

class VideoPlayer
{
private:
    /* data */
public:
    bool video_loop = true;
    void play(FILE *f);
    void playBuffer(const uint8_t *video_buffer, uint32_t buffer_size);
};

extern VideoPlayer videoPlayer;
LV_IMG_DECLARE(moon_000);
LV_IMG_DECLARE(moon_010);
LV_IMG_DECLARE(moon_020);
LV_IMG_DECLARE(moon_030);
LV_IMG_DECLARE(moon_040);
LV_IMG_DECLARE(moon_050);
LV_IMG_DECLARE(moon_060);
LV_IMG_DECLARE(moon_070);
LV_IMG_DECLARE(moon_080);
LV_IMG_DECLARE(moon_090);
LV_IMG_DECLARE(moon_100);
LV_IMG_DECLARE(moon_110);
LV_IMG_DECLARE(moon_120);
LV_IMG_DECLARE(moon_130);
LV_IMG_DECLARE(moon_140);
LV_IMG_DECLARE(moon_150);
LV_IMG_DECLARE(moon_160);
LV_IMG_DECLARE(moon_170);
LV_IMG_DECLARE(moon_180);
LV_IMG_DECLARE(moon_190);
LV_IMG_DECLARE(moon_200);
LV_IMG_DECLARE(moon_210);
LV_IMG_DECLARE(moon_220);
LV_IMG_DECLARE(moon_230);
LV_IMG_DECLARE(moon_240);
LV_IMG_DECLARE(moon_250);
LV_IMG_DECLARE(moon_260);
LV_IMG_DECLARE(moon_270);
LV_IMG_DECLARE(moon_280);
LV_IMG_DECLARE(moon_290);
LV_IMG_DECLARE(moon_300);
LV_IMG_DECLARE(moon_310);
LV_IMG_DECLARE(moon_320);
LV_IMG_DECLARE(moon_330);
LV_IMG_DECLARE(moon_340);
LV_IMG_DECLARE(moon_350);