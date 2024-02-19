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