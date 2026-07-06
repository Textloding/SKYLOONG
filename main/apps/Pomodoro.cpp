#include "A_Config.h"

extern bool consumePomodoroConfirmRequest();

enum PomodoroPhase : uint8_t
{
    POMODORO_FOCUS = 0,
    POMODORO_SHORT_BREAK = 1,
    POMODORO_LONG_BREAK = 2,
};

static lv_obj_t *arc_timer = NULL;
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
static bool waiting_confirm = false;

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
        return "SHORT BREAK";
    if (p == POMODORO_LONG_BREAK)
        return "LONG BREAK";
    return "FOCUS";
}

static const char *phaseBadge(PomodoroPhase p)
{
    if (p == POMODORO_SHORT_BREAK)
        return "REST";
    if (p == POMODORO_LONG_BREAK)
        return "RESET";
    return "WORK";
}

static void beginPhaseLocked(PomodoroPhase next)
{
    phase = next;
    phase_started_ms = millis();
    phase_duration_ms = durationForPhase(phase);
    waiting_confirm = false;
    last_tone_ms = 0;
    last_render_second = 0xFFFFFFFF;
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

static void renderPomodoro(uint32_t remaining_ms, PomodoroPhase current_phase, uint8_t round, bool waiting, uint32_t duration_ms)
{
    if (lbl_time_left == NULL)
        return;

    uint32_t seconds = (remaining_ms + 999) / 1000;
    if (waiting)
        seconds = 0;

    if (seconds == last_render_second)
        return;
    last_render_second = seconds;

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

    hal.LOCKLV();
    lv_label_set_text_fmt(lbl_time_left, "%02" PRIu32 ":%02" PRIu32, minutes, rest);
    lv_label_set_text(lbl_phase, waiting ? "TIME IS UP" : phaseTitle(current_phase));
    lv_label_set_text(lbl_badge, phaseBadge(current_phase));
    lv_label_set_text_fmt(lbl_round, "Round %u", round + 1);
    lv_label_set_text(lbl_hint, waiting ? "Press fn+~ to stop sound and continue" : "Timer keeps running in background");
    lv_arc_set_value(arc_timer, progress);
    lv_obj_set_style_arc_color(arc_timer, waiting ? lv_color_hex(0xff5f6d) : lv_color_hex(0x41d3bd), LV_PART_INDICATOR);
    lv_obj_set_style_text_color(lbl_time_left, waiting ? lv_color_hex(0xffedf0) : lv_color_hex(0xf7fbff), 0);
    hal.UNLOCKLV();
}

void AppPomodoro::setup()
{
    pomodoro_init();

    hal.LOCKLV();
    lv_obj_clear_flag(_appScreen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(_appScreen, lv_color_hex(0x111827), 0);
    lv_obj_set_style_text_color(_appScreen, lv_color_hex(0xf7fbff), 0);
    lv_obj_set_style_text_font(_appScreen, &lv_font_chinese_16, 0);

    lv_obj_t *panel = lv_obj_create(_appScreen);
    lv_obj_set_size(panel, 300, 220);
    lv_obj_center(panel);
    lv_obj_set_style_radius(panel, 10, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x172033), 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x2f3a52), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    lbl_badge = lv_label_create(panel);
    lv_label_set_text(lbl_badge, phaseBadge(phase));
    lv_obj_set_style_text_font(lbl_badge, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(lbl_badge, lv_color_hex(0x41d3bd), 0);
    lv_obj_align(lbl_badge, LV_ALIGN_TOP_LEFT, 6, 4);

    lbl_round = lv_label_create(panel);
    lv_label_set_text(lbl_round, "Round 1");
    lv_obj_set_style_text_color(lbl_round, lv_color_hex(0xaab4c4), 0);
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
    lv_obj_set_style_arc_color(arc_timer, lv_color_hex(0x2d364a), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_timer, lv_color_hex(0x41d3bd), LV_PART_INDICATOR);

    lbl_time_left = lv_label_create(panel);
    lv_label_set_text(lbl_time_left, "25:00");
    lv_obj_set_style_text_font(lbl_time_left, &lv_font_montserrat_48, 0);
    lv_obj_center(lbl_time_left);

    lbl_phase = lv_label_create(panel);
    lv_label_set_text(lbl_phase, phaseTitle(phase));
    lv_obj_set_style_text_color(lbl_phase, lv_color_hex(0xaab4c4), 0);
    lv_obj_align(lbl_phase, LV_ALIGN_BOTTOM_MID, 0, -34);

    lbl_hint = lv_label_create(panel);
    lv_label_set_text(lbl_hint, "Timer keeps running in background");
    lv_obj_set_style_text_color(lbl_hint, lv_color_hex(0x7f8ba3), 0);
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
    arc_timer = NULL;
    lbl_time_left = NULL;
    lbl_phase = NULL;
    lbl_round = NULL;
    lbl_hint = NULL;
    lbl_badge = NULL;
}
