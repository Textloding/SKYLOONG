#include "A_Config.h"
#include "GUI.h"

#define TOAST_FADE_IN_TIME 0
#define TOAST_FADE_OUT_TIME 0
#define TOAST_DISPLAY_TIME 2500
#define TOAST_OPA (255)
#define TOAST_UP_DISTANCE 16
static lv_obj_t *last_toast;

static void toast_anim_out(lv_obj_t *obj)
{
    lv_anim_t a;
    uint16_t p;
    last_toast = NULL;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    p = lv_obj_get_style_y(obj, 0);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_values(&a, p, p + TOAST_UP_DISTANCE);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_time(&a, TOAST_FADE_OUT_TIME);
    lv_anim_set_delay(&a, 0);
    lv_anim_start(&a);
    lv_obj_fade_out(obj, TOAST_FADE_OUT_TIME * 3 / 5, TOAST_FADE_OUT_TIME * 2 / 5);

    lv_anim_set_exec_cb(&a, NULL);
    lv_anim_set_ready_cb(&a, lv_obj_del_anim_ready_cb);
    lv_anim_set_delay(&a, TOAST_FADE_OUT_TIME);
    lv_anim_set_time(&a, 1);
    lv_anim_start(&a);
}

static void toast_anim_in(lv_obj_t *obj, uint32_t delay)
{
    lv_anim_t a;
    uint16_t p;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    p = lv_obj_get_style_y(obj, 0);
    lv_anim_set_values(&a, p + TOAST_UP_DISTANCE, p);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_time(&a, TOAST_FADE_IN_TIME);
    lv_anim_set_delay(&a, 0);
    lv_anim_start(&a);

    lv_anim_set_values(&a, 0, TOAST_OPA);
    lv_anim_set_exec_cb(&a, [](void *obj, int32_t v)
                        { lv_obj_set_style_opa((lv_obj_t *)obj, v, 0); });
    lv_anim_set_time(&a, TOAST_FADE_IN_TIME);
    lv_anim_set_delay(&a, 0);
    lv_anim_start(&a);

    lv_anim_set_exec_cb(&a, NULL);
    lv_anim_set_ready_cb(&a, [](lv_anim_t *a)
                         { if(last_toast == a->var) toast_anim_out((lv_obj_t *)a->var); }); // 如果上个toast已经开始fade就不重复
    lv_anim_set_time(&a, 1);
    lv_anim_set_delay(&a, TOAST_DISPLAY_TIME);
    lv_anim_start(&a);
}

void GUI::toast(const char *str, bool lock, uint32_t delay)
{
    if (str == NULL)
        return;
    if (str[0] == 0)
        return;
    if (lock)
        hal.LOCKLV();
    if(delay == 0)
        delay = TOAST_DISPLAY_TIME;
    if (lv_obj_is_valid(last_toast))
    {
        toast_anim_out(last_toast);
    }
    lv_obj_t *obj_toast = lv_obj_create(lv_layer_sys());
    lv_obj_set_size(obj_toast, screenWidth / 2, LV_SIZE_CONTENT);
    lv_obj_set_style_opa(obj_toast, 0, 0);
    lv_obj_set_style_radius(obj_toast, 30, 0);
    lv_obj_set_style_border_width(obj_toast, 0, 0);
    lv_obj_set_style_bg_opa(obj_toast, LV_OPA_COVER - 80, 0);
    lv_obj_t *lbl = lv_label_create(obj_toast);
    lv_label_set_text(lbl, str);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_size(lbl, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(lbl);
    lv_obj_set_style_text_font(lbl, &lv_font_chinese_16, false);
    lv_obj_clear_flag(obj_toast, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_align(obj_toast, LV_ALIGN_BOTTOM_MID, 0, -(screenHeight / 4));
    toast_anim_in(obj_toast,delay);
    last_toast = obj_toast;
    if (lock)
        hal.UNLOCKLV();
}
