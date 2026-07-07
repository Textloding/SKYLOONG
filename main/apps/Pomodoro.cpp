#include "A_Config.h"

extern bool consumePomodoroConfirmRequest();

struct PomodoroTheme
{
    uint32_t screen_bg;
    uint32_t panel_bg;
    uint32_t panel_border;
    uint32_t arc_bg;
    uint32_t accent;
    uint32_t waiting;
    uint32_t text;
    uint32_t muted;
    uint32_t faint;
};

static const PomodoroTheme pomodoro_themes[] = {
    {0x1f0f12, 0x33191d, 0x6b2832, 0x5a222a, 0xff6b7a, 0xffc857, 0xfff7f5, 0xf5b7ba, 0xb98086},
    {0x081827, 0x0d2233, 0x22506a, 0x17384d, 0x41d3bd, 0xff7a8a, 0xf7fbff, 0xa6c9d8, 0x7192a4},
    {0x0d1d16, 0x13291d, 0x2e6444, 0x214d34, 0x7ee787, 0xffc857, 0xf5fff7, 0xb8d8bd, 0x7fa987},
};

enum PomodoroPhase : uint8_t
{
    POMODORO_FOCUS = 0,
    POMODORO_SHORT_BREAK = 1,
    POMODORO_LONG_BREAK = 2,
};

static lv_obj_t *screen_timer = NULL;
static lv_obj_t *arc_timer = NULL;
static lv_obj_t *panel_timer = NULL;
static lv_obj_t *lbl_time_left = NULL;
static lv_obj_t *lbl_phase = NULL;
static lv_obj_t *lbl_round = NULL;
static lv_obj_t *lbl_hint = NULL;
static lv_obj_t *lbl_badge = NULL;

static SemaphoreHandle_t pomodoro_mutex = NULL;
static TaskHandle_t pomodoro_task = NULL;
static PomodoroPhase phase = POMODORO_FOCUS;
static uint8_t completed_focus_rounds = 0;
static uint32_t phase_started_ms = 0;
static uint32_t phase_duration_ms = 0;
static uint32_t last_tone_ms = 0;
static uint32_t last_render_second = 0xFFFFFFFF;
static uint8_t last_render_theme = 0xFF;
static bool waiting_confirm = false;

static const PomodoroTheme &activeTheme()
{
    uint8_t index = hal.pomodoro_theme;
    if (index >= sizeof(pomodoro_themes) / sizeof(pomodoro_themes[0]))
        index = 0;
    return pomodoro_themes[index];
}

static uint32_t minutesToMs(uint8_t minutes)
{
    uint32_t safe_minutes = minutes == 0 ? 1 : minutes;
    return safe_minutes * 60UL * 1000UL;
}

static uint32_t durationForPhase(PomodoroPhase p)
{
    if (p == POMODORO_SHORT_BREAK)
        return minutesToMs(hal.pomodoro_short_break_min);
    if (p == POMODORO_LONG_BREAK)
        return minutesToMs(hal.pomodoro_long_break_min);
    return minutesToMs(hal.pomodoro_focus_min);
}

static const char *phaseTitle(PomodoroPhase p)
{
    if (p == POMODORO_SHORT_BREAK)
        return "短休息";
    if (p == POMODORO_LONG_BREAK)
        return "长休息";
    return "专注中";
}

static const char *phaseBadge(PomodoroPhase p)
{
    if (p == POMODORO_SHORT_BREAK)
        return "休息";
    if (p == POMODORO_LONG_BREAK)
        return "长休";
    return "专注";
}

static void beginPhaseLocked(PomodoroPhase next)
{
    phase = next;
    phase_started_ms = millis();
    phase_duration_ms = durationForPhase(phase);
    waiting_confirm = false;
    last_tone_ms = 0;
    last_render_second = 0xFFFFFFFF;
    last_render_theme = 0xFF;
    hal.setPomodoroWaiting(false);
}

static PomodoroPhase nextPhaseLocked()
{
    if (phase == POMODORO_FOCUS)
    {
        completed_focus_rounds++;
        uint8_t every = hal.pomodoro_long_break_every == 0 ? 4 : hal.pomodoro_long_break_every;
        return (completed_focus_rounds % every == 0) ? POMODORO_LONG_BREAK : POMODORO_SHORT_BREAK;
    }
    return POMODORO_FOCUS;
}

static uint32_t remainingMsLocked(uint32_t now)
{
    uint32_t elapsed = now - phase_started_ms;
    if (elapsed >= phase_duration_ms)
        return 0;
    return phase_duration_ms - elapsed;
}

static void pomodoroTask(void *p)
{
    while (1)
    {
        if (hal.pomodoro_enable)
        {
            uint32_t now = millis();
            bool play_tone = false;

            xSemaphoreTake(pomodoro_mutex, portMAX_DELAY);
            if (phase_duration_ms == 0)
                beginPhaseLocked(POMODORO_FOCUS);

            uint32_t remaining = remainingMsLocked(now);
            if (!waiting_confirm && remaining == 0)
            {
                waiting_confirm = true;
                last_tone_ms = 0;
                hal.setPomodoroWaiting(true);
                if (hal.pomodoro_auto_switch)
                    hal.requestPomodoroAutoSwitch();
            }

            if (waiting_confirm)
            {
                if (consumePomodoroConfirmRequest())
                {
                    beginPhaseLocked(nextPhaseLocked());
                }
                else if (last_tone_ms == 0 || now - last_tone_ms > 3500)
                {
                    last_tone_ms = now;
                    play_tone = true;
                }
            }
            xSemaphoreGive(pomodoro_mutex);

            if (play_tone)
                hal.playPomodoroTone();
        }
        else
        {
            hal.setPomodoroWaiting(false);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void pomodoro_init()
{
    if (pomodoro_mutex == NULL)
        pomodoro_mutex = xSemaphoreCreateMutex();
    if (phase_duration_ms == 0)
    {
        xSemaphoreTake(pomodoro_mutex, portMAX_DELAY);
        beginPhaseLocked(POMODORO_FOCUS);
        xSemaphoreGive(pomodoro_mutex);
    }
    if (pomodoro_task == NULL)
        xTaskCreate(pomodoroTask, "task_pomodoro", 4096, NULL, 2, &pomodoro_task);
}

void pomodoro_reset_current()
{
    pomodoro_init();
    bool should_stop_audio = false;
    xSemaphoreTake(pomodoro_mutex, portMAX_DELAY);
    should_stop_audio = waiting_confirm;
    beginPhaseLocked(phase);
    xSemaphoreGive(pomodoro_mutex);
    if (should_stop_audio)
        hal.audio_stop();
}

static void renderPomodoro(uint32_t remaining_ms, PomodoroPhase current_phase, uint8_t round, bool waiting, uint32_t duration_ms)
{
    if (lbl_time_left == NULL)
        return;

    uint32_t seconds = (remaining_ms + 999) / 1000;
    if (waiting)
        seconds = 0;

    if (seconds == last_render_second && hal.pomodoro_theme == last_render_theme)
        return;
    last_render_second = seconds;
    last_render_theme = hal.pomodoro_theme;

    uint32_t minutes = seconds / 60;
    uint32_t rest = seconds % 60;
    int32_t progress = 0;
    if (duration_ms > 0)
        progress = (int32_t)((duration_ms - remaining_ms) * 100 / duration_ms);
    if (waiting)
        progress = 100;
    if (progress < 0)
        progress = 0;
    if (progress > 100)
        progress = 100;

    const PomodoroTheme &theme = activeTheme();

    hal.LOCKLV();
    if (screen_timer != NULL)
        lv_obj_set_style_bg_color(screen_timer, lv_color_hex(theme.screen_bg), 0);
    if (panel_timer != NULL)
    {
        lv_obj_set_style_bg_color(panel_timer, lv_color_hex(theme.panel_bg), 0);
        lv_obj_set_style_border_color(panel_timer, lv_color_hex(theme.panel_border), 0);
    }
    lv_label_set_text_fmt(lbl_time_left, "%02" PRIu32 ":%02" PRIu32, minutes, rest);
    lv_label_set_text(lbl_phase, waiting ? "时间到" : phaseTitle(current_phase));
    lv_label_set_text(lbl_badge, phaseBadge(current_phase));
    lv_label_set_text_fmt(lbl_round, "第 %u 轮", round + 1);
    lv_label_set_text(lbl_hint, waiting ? "按 fn+~ 停止提示音并进入下一段" : "后台持续计时");
    lv_arc_set_value(arc_timer, progress);
    lv_obj_set_style_arc_color(arc_timer, lv_color_hex(theme.arc_bg), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_timer, lv_color_hex(waiting ? theme.waiting : theme.accent), LV_PART_INDICATOR);
    lv_obj_set_style_text_color(lbl_time_left, lv_color_hex(theme.text), 0);
    lv_obj_set_style_text_color(lbl_phase, lv_color_hex(waiting ? theme.waiting : theme.muted), 0);
    lv_obj_set_style_text_color(lbl_badge, lv_color_hex(waiting ? theme.waiting : theme.accent), 0);
    lv_obj_set_style_text_color(lbl_round, lv_color_hex(theme.muted), 0);
    lv_obj_set_style_text_color(lbl_hint, lv_color_hex(theme.faint), 0);
    hal.UNLOCKLV();
}

void AppPomodoro::setup()
{
    pomodoro_init();

    hal.LOCKLV();
    screen_timer = _appScreen;
    const PomodoroTheme &theme = activeTheme();
    lv_obj_clear_flag(_appScreen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(_appScreen, lv_color_hex(theme.screen_bg), 0);
    lv_obj_set_style_text_color(_appScreen, lv_color_hex(theme.text), 0);
    lv_obj_set_style_text_font(_appScreen, &lv_font_chinese_16, 0);

    lv_obj_t *panel = lv_obj_create(_appScreen);
    panel_timer = panel;
    lv_obj_set_size(panel, 300, 220);
    lv_obj_center(panel);
    lv_obj_set_style_radius(panel, 10, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(theme.panel_bg), 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(theme.panel_border), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    lbl_badge = lv_label_create(panel);
    lv_label_set_text(lbl_badge, phaseBadge(phase));
    lv_obj_set_style_text_font(lbl_badge, &lv_font_chinese_16, 0);
    lv_obj_set_style_text_color(lbl_badge, lv_color_hex(theme.accent), 0);
    lv_obj_align(lbl_badge, LV_ALIGN_TOP_LEFT, 6, 4);

    lbl_round = lv_label_create(panel);
    lv_label_set_text(lbl_round, "第 1 轮");
    lv_obj_set_style_text_color(lbl_round, lv_color_hex(theme.muted), 0);
    lv_obj_align(lbl_round, LV_ALIGN_TOP_RIGHT, -6, 7);

    arc_timer = lv_arc_create(panel);
    lv_obj_set_size(arc_timer, 142, 142);
    lv_obj_align(arc_timer, LV_ALIGN_CENTER, 0, 2);
    lv_arc_set_range(arc_timer, 0, 100);
    lv_arc_set_value(arc_timer, 0);
    lv_obj_remove_style(arc_timer, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc_timer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(arc_timer, 10, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_timer, 10, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_timer, lv_color_hex(theme.arc_bg), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_timer, lv_color_hex(theme.accent), LV_PART_INDICATOR);

    lbl_time_left = lv_label_create(panel);
    lv_label_set_text(lbl_time_left, "25:00");
    lv_obj_set_style_text_font(lbl_time_left, &lv_font_montserrat_48, 0);
    lv_obj_center(lbl_time_left);

    lbl_phase = lv_label_create(panel);
    lv_label_set_text(lbl_phase, phaseTitle(phase));
    lv_obj_set_style_text_color(lbl_phase, lv_color_hex(theme.muted), 0);
    lv_obj_set_width(lbl_phase, 260);
    lv_label_set_long_mode(lbl_phase, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(lbl_phase, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(lbl_phase, LV_ALIGN_BOTTOM_MID, 0, -34);

    lbl_hint = lv_label_create(panel);
    lv_label_set_text(lbl_hint, "后台持续计时");
    lv_obj_set_style_text_color(lbl_hint, lv_color_hex(theme.faint), 0);
    lv_obj_set_width(lbl_hint, 278);
    lv_label_set_long_mode(lbl_hint, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(lbl_hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(lbl_hint, LV_ALIGN_BOTTOM_MID, 0, -10);
    hal.UNLOCKLV();

    last_render_second = 0xFFFFFFFF;
}

void AppPomodoro::loop()
{
    if (!hal.pomodoro_enable)
    {
        xSemaphoreGive(appManagerLite._binary_switchApp);
        return;
    }

    uint32_t now = millis();
    uint32_t remaining = 0;
    uint32_t duration = 0;
    PomodoroPhase current_phase = POMODORO_FOCUS;
    uint8_t round = 0;
    bool waiting = false;

    xSemaphoreTake(pomodoro_mutex, portMAX_DELAY);
    remaining = remainingMsLocked(now);
    duration = phase_duration_ms;
    current_phase = phase;
    round = completed_focus_rounds;
    waiting = waiting_confirm;
    xSemaphoreGive(pomodoro_mutex);

    renderPomodoro(remaining, current_phase, round, waiting, duration);
    delay(40);
}

void AppPomodoro::destroy()
{
    screen_timer = NULL;
    arc_timer = NULL;
    panel_timer = NULL;
    lbl_time_left = NULL;
    lbl_phase = NULL;
    lbl_round = NULL;
    lbl_hint = NULL;
    lbl_badge = NULL;
}
