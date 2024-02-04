#include "A_Config.h"
#include "GUI.h"

#define TOAST_FADE_IN_TIME 300
#define TOAST_FADE_OUT_TIME 300
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

static void toast_anim_in(lv_obj_t *obj)
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

void GUI::toast(const char *str, bool lock)
{
    if (str == NULL)
        return;
    if (str[0] == 0)
        return;
    if (lock)
        hal.LOCKLV();
    if (lv_obj_is_valid(last_toast))
    {
        toast_anim_out(last_toast);
    }
    lv_obj_t *obj_toast = lv_obj_create(lv_layer_sys());
    lv_obj_set_size(obj_toast, screenWidth / 2, LV_SIZE_CONTENT);
    lv_obj_set_style_opa(obj_toast, 0, 0);
    lv_obj_set_style_radius(obj_toast, 30, 0);
    lv_obj_set_style_border_width(obj_toast, 0, 0);
    //lv_obj_set_style_bg_color(obj_toast, lv_palette_lighten(LV_PALETTE_GREY, 2), 0);
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
    toast_anim_in(obj_toast);
    last_toast = obj_toast;
    if (lock)
        hal.UNLOCKLV();
}
static lv_style_t style_status_obj_default;
static lv_style_t style_status_obj_loading;
static lv_style_t style_status_obj_green;
static lv_style_t style_status_obj_red;
lv_obj_t *arr_status_obj[10];
int status_obj_count = 0;
lv_obj_t* status_obj_parent = NULL;
static lv_obj_t* status_bar_create(const char* symbol)
{
    lv_obj_t* obj_status = lv_obj_create(status_obj_parent);
    lv_obj_set_size(obj_status, 28, 28);
    lv_obj_set_style_radius(obj_status, 10, 0);
    lv_obj_clear_flag(obj_status, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_font(obj_status, &symbol_16, 0);
    lv_obj_t* lbl_status = lv_label_create(obj_status);
    lv_label_set_text(lbl_status, symbol);
    lv_obj_align(lbl_status, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(obj_status, &style_status_obj_default, 0);
    lv_obj_add_style(obj_status, &style_status_obj_loading, LV_STATE_USER_1);
    lv_obj_add_style(obj_status, &style_status_obj_green, LV_STATE_USER_2);
    lv_obj_add_style(obj_status, &style_status_obj_red, LV_STATE_USER_3);
    lv_obj_set_x(obj_status, status_obj_count * 30);
    return obj_status;
}
static void status_bar_refresh()
{
    int first_move = -1;
    int current_delay = 0;
    int shift = 0;
    for (int i = 0; i < 10; i++)
    {
        if (arr_status_obj[i] == NULL)
        {
            if (first_move == -1)
            {
                first_move = i;
            }
            ++shift;
        }
        else
        {
            arr_status_obj[i - shift] = arr_status_obj[i];
        }
    }
    for (int i = status_obj_count; i < 10; i++)
    {
        arr_status_obj[i] = NULL;
    }
    for (int i = first_move; i < 10; i++)
    {
        if (arr_status_obj[i] != NULL)
        {
            lv_obj_move_anim(arr_status_obj[i], i * 30, 0x7fff, 500, current_delay);
            current_delay += 60;
        }
    }
}
lv_obj_t* GUI::status_bar_add(const char* symbol, int delay_ms = 0)
{
    if(status_obj_count >= 10)
        return NULL;
    lv_obj_t *obj_status = status_bar_create(symbol);
    arr_status_obj[status_obj_count++] = obj_status;
    lv_obj_push_down(obj_status, 40, 500, delay_ms);
    return obj_status;
}
void GUI::status_bar_remove(lv_obj_t* obj)
{
    if(obj == NULL)
        return;
    for (int i = 0; i < 10; i++)
    {
        if (arr_status_obj[i] == obj)
        {
            arr_status_obj[i] = NULL;
            lv_obj_fall_down(obj, 40, 500);
            lv_obj_del_delayed(obj, 500);
            --status_obj_count;
            status_bar_refresh();
            return;
        }
    }
}
void GUI::status_bar_status_set(lv_obj_t* obj, lv_state_t status)
{
    if (obj == NULL)
        return;
    lv_obj_clear_state(obj, LV_STATE_USER_1 | LV_STATE_USER_2 | LV_STATE_USER_3 | LV_STATE_USER_4);
    if(status != 0)
        lv_obj_add_state(obj, status);
}
lv_obj_t *GUI::status_bar_insert(const char *symbol, int index)
{
    int current_delay = 0;
    if (status_obj_count >= 10)
        return NULL;
    if (index < 0 || index > 10)
        return NULL;
    lv_obj_t *obj = status_bar_create(symbol);
    current_delay = (status_obj_count - index) * 60;
    for (int i = status_obj_count; i > index; i--)
    {
        arr_status_obj[i] = arr_status_obj[i - 1];
        lv_obj_move_anim(arr_status_obj[i], i * 30, 0x7fff, 500, current_delay);
        current_delay -= 60;
    }
    arr_status_obj[index] = obj;
    lv_obj_set_x(obj, index * 30);
    lv_obj_push_down(obj, 40, 500, 0);
    ++status_obj_count;
    return obj;
}
lv_obj_t *GUI::status_bar_replace(const char *symbol, lv_obj_t *&last)
{
    if (last == NULL)
    {
        last = status_bar_add(symbol);
        return last;
    }
    for (int i = 0; i < 10; i++)
    {
        if (arr_status_obj[i] == last)
        {
            lv_obj_t *obj = status_bar_insert(symbol, i);
            status_bar_remove(last);
            last = obj;
            return last;
        }
    }
    return last;
}
void GUI::status_bar_init()
{
    // 顶层状态栏
    lv_obj_t *box_status = lv_obj_create(lv_layer_top());
    lv_obj_set_size(box_status, 320, 32);
    lv_obj_set_style_radius(box_status, 0, 0);
    lv_obj_set_style_bg_opa(box_status, 0, 0);
    lv_obj_set_style_border_width(box_status, 0, 0);
    lv_obj_set_style_pad_column(box_status, 2, 0);
    lv_obj_set_style_pad_all(box_status, 2, 0);
    lv_obj_clear_flag(box_status, LV_OBJ_FLAG_SCROLLABLE);

    static const lv_style_prop_t trans_props[] = {
                                            LV_STYLE_BG_COLOR,
                                            LV_STYLE_PROP_INV, /*End marker*/
    };
    static lv_style_transition_dsc_t trans_def;
    lv_style_transition_dsc_init(&trans_def, trans_props, lv_anim_path_linear, 300, 0, NULL);

    lv_style_init(&style_status_obj_default);
    lv_style_init(&style_status_obj_loading);
    lv_style_init(&style_status_obj_green);
    lv_style_init(&style_status_obj_red);
    lv_style_set_bg_color(&style_status_obj_loading, lv_palette_darken(LV_PALETTE_BLUE, 1));
    lv_style_set_bg_color(&style_status_obj_green, lv_palette_darken(LV_PALETTE_GREEN, 1));
    lv_style_set_bg_color(&style_status_obj_red, lv_palette_darken(LV_PALETTE_RED, 1));
    lv_style_set_transition(&style_status_obj_default, &trans_def);
    lv_style_set_transition(&style_status_obj_loading, &trans_def);
    lv_style_set_transition(&style_status_obj_green, &trans_def);
    lv_style_set_transition(&style_status_obj_red, &trans_def);
    status_obj_parent = box_status;

    lv_timer_t *tm = lv_timer_create([](lv_timer_t* timer) {
        for (int i = 0; i < 10; i++)
        {
            if (arr_status_obj[i] != NULL)
            {
                if (lv_obj_has_state(arr_status_obj[i], LV_STATE_USER_1))
                {
                    lv_obj_add_state(arr_status_obj[i], LV_STATE_USER_4);
                    lv_obj_clear_state(arr_status_obj[i], LV_STATE_USER_1);
                }
                else if (lv_obj_has_state(arr_status_obj[i], LV_STATE_USER_4))
                {
                    lv_obj_add_state(arr_status_obj[i], LV_STATE_USER_1);
                    lv_obj_clear_state(arr_status_obj[i], LV_STATE_USER_4);
                }
            }
        }
        }, 520, NULL);
    lv_obj_add_event_cb(box_status, [](lv_event_t* e) {
        lv_timer_del((lv_timer_t*)e->user_data);
        }, LV_EVENT_DELETE, tm);
    
    /////////////////////////////////////////////////////////////
    return;
}
static bool status_bar_showing = true;
void GUI::status_bar_show(bool show)
{
    if(show == status_bar_showing)
        return;
    status_bar_showing = show;
    hal.LOCKLV();
    if(show)
    {
        lv_obj_move_anim(status_obj_parent, 0x7fff, 0, 500, 0);
    }
    else
    {
        lv_obj_move_anim(status_obj_parent, 0x7fff, -40, 500, 0);
    }
    hal.UNLOCKLV();
}
