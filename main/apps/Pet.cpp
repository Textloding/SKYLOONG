#include "A_Config.h"
#include "ESP_I2S.h"
#include "pet_bark_sound.h"

extern I2SClass i2s;

extern "C" {
    extern const lv_img_dsc_t pet_dog_bark_0;
    extern const lv_img_dsc_t pet_dog_bark_1;
    extern const lv_img_dsc_t pet_dog_bark_2;
    extern const lv_img_dsc_t pet_dog_bark_3;
    extern const lv_img_dsc_t pet_dog_walk_0;
    extern const lv_img_dsc_t pet_dog_walk_1;
    extern const lv_img_dsc_t pet_dog_walk_2;
    extern const lv_img_dsc_t pet_dog_walk_3;
    extern const lv_img_dsc_t pet_dog_run_0;
    extern const lv_img_dsc_t pet_dog_run_1;
    extern const lv_img_dsc_t pet_dog_run_2;
    extern const lv_img_dsc_t pet_dog_run_3;
    extern const lv_img_dsc_t pet_dog_sit_0;
    extern const lv_img_dsc_t pet_dog_sit_1;
    extern const lv_img_dsc_t pet_dog_sit_2;
    extern const lv_img_dsc_t pet_dog_sit_3;
    extern const lv_img_dsc_t pet_dog_idle_0;
    extern const lv_img_dsc_t pet_dog_idle_1;
    extern const lv_img_dsc_t pet_dog_idle_2;
    extern const lv_img_dsc_t pet_dog_idle_3;
}

static lv_obj_t *pet_stage = NULL;
static lv_obj_t *pet_ground = NULL;
static lv_obj_t *pet_energy_track = NULL;
static lv_obj_t *pet_energy_fill = NULL;
static lv_obj_t *pet_shadow = NULL;
static lv_obj_t *pet_dog_img = NULL;
static lv_obj_t *pet_bursts[6] = {NULL};
static lv_obj_t *pet_keys[5] = {NULL};
static lv_obj_t *lbl_pet_title = NULL;
static lv_obj_t *lbl_pet_status = NULL;
static lv_obj_t *lbl_pet_hint = NULL;

static uint32_t last_render_ms = 0;
static uint32_t last_decay_ms = 0;
static uint32_t last_key_seq = 0;
static uint32_t last_key_ms = 0;
static uint32_t last_bark_sound_ms = 0;
static uint16_t pet_energy = 0;
static uint16_t combo = 0;
static const lv_img_dsc_t *last_pet_frame = NULL;

static const uint16_t PET_BARK_COOLDOWN_MS = 3200;

struct PetTheme
{
    uint32_t screen_bg;
    uint32_t sleep_bg;
    uint32_t grid;
    uint32_t ground;
    uint32_t accent;
    uint32_t focused;
    uint32_t excited;
    uint32_t sleep;
    uint32_t track;
    uint32_t key_idle;
    uint32_t title;
    uint32_t text;
    uint32_t faint;
};

static const PetTheme pet_themes[] = {
    {0x07111F, 0x080B18, 0x10213A, 0x0F172A, 0x38BDF8, 0x34D399, 0xFACC15, 0x818CF8, 0x1E293B, 0x172033, 0xBAE6FD, 0xE5F2FF, 0x94A3B8},
    {0x061B18, 0x08140F, 0x0E2D27, 0x10231F, 0x2DD4BF, 0x86EFAC, 0xFDE68A, 0x6EE7B7, 0x16352F, 0x102820, 0x99F6E4, 0xECFEFF, 0x9CA3AF},
    {0x201016, 0x130A12, 0x37192B, 0x2A1720, 0xFB7185, 0xF9A8D4, 0xFDE047, 0xC084FC, 0x3A2030, 0x2A1724, 0xFBCFE8, 0xFFF7FB, 0xF0ABFC},
    {0x100B1D, 0x080712, 0x251A42, 0x171228, 0xA78BFA, 0x22D3EE, 0xFDE047, 0x818CF8, 0x241E3D, 0x1B1431, 0xDDD6FE, 0xF5F3FF, 0xC4B5FD},
};

static const lv_img_dsc_t *const pet_dog_idle_frames[] = {
    &pet_dog_idle_0, &pet_dog_idle_1, &pet_dog_idle_2, &pet_dog_idle_3,
};

static const lv_img_dsc_t *const pet_dog_walk_frames[] = {
    &pet_dog_walk_0, &pet_dog_walk_1, &pet_dog_walk_2, &pet_dog_walk_3,
};

static const lv_img_dsc_t *const pet_dog_run_frames[] = {
    &pet_dog_run_0, &pet_dog_run_1, &pet_dog_run_2, &pet_dog_run_3,
};

static const lv_img_dsc_t *const pet_dog_bark_frames[] = {
    &pet_dog_bark_0, &pet_dog_bark_1, &pet_dog_bark_2, &pet_dog_bark_3,
};

static const lv_img_dsc_t *const pet_dog_sit_frames[] = {
    &pet_dog_sit_0, &pet_dog_sit_1, &pet_dog_sit_2, &pet_dog_sit_3,
};

enum PetDogMood
{
    PET_DOG_IDLE,
    PET_DOG_WALK,
    PET_DOG_RUN,
    PET_DOG_BARK,
    PET_DOG_SIT,
};

static PetDogMood last_pet_mood = PET_DOG_IDLE;

static const PetTheme &petTheme()
{
    uint8_t index = hal.pet_theme;
    if (index >= sizeof(pet_themes) / sizeof(pet_themes[0]))
        index = 0;
    return pet_themes[index];
}

static const char *petDisplayName()
{
    return strlen(hal.pet_name) > 0 ? hal.pet_name : "键盘宠物";
}

static lv_obj_t *createBox(lv_obj_t *parent, int16_t w, int16_t h, uint32_t color, uint8_t radius, lv_opa_t opa = LV_OPA_COVER)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(obj, opa, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}

static void setBox(lv_obj_t *obj, int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color, uint8_t radius, lv_opa_t opa = LV_OPA_COVER)
{
    if (obj == NULL)
        return;
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(obj, opa, 0);
}

static void showBox(lv_obj_t *obj, bool visible)
{
    if (obj == NULL)
        return;
    if (visible)
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
}

static void setTextColor(lv_obj_t *obj, uint32_t color)
{
    if (obj != NULL)
        lv_obj_set_style_text_color(obj, lv_color_hex(color), 0);
}

static uint16_t addCap(uint16_t value, uint32_t add, uint16_t cap)
{
    uint32_t next = value + add;
    return next > cap ? cap : (uint16_t)next;
}

static PetDogMood petMoodForState(bool sleepy, bool waiting, bool excited, bool focused)
{
    if (sleepy)
        return PET_DOG_SIT;
    if (excited && combo >= 70)
        return PET_DOG_BARK;
    if (excited)
        return PET_DOG_RUN;
    if (focused)
        return PET_DOG_WALK;
    if (waiting)
        return PET_DOG_IDLE;
    return PET_DOG_IDLE;
}

static PetDogMood currentPetMood(uint32_t now)
{
    uint32_t idle_ms = now - last_key_ms;
    bool sleepy = idle_ms > 90000;
    bool waiting = idle_ms > 20000 && !sleepy;
    bool excited = combo >= 45 || pet_energy > 76;
    bool focused = combo >= 15 || pet_energy > 35;
    return petMoodForState(sleepy, waiting, excited, focused);
}

static const lv_img_dsc_t *petFrameAt(const lv_img_dsc_t *const *frames, size_t count, uint32_t now, uint16_t frame_ms)
{
    if (count == 0)
        return &pet_dog_idle_0;
    return frames[(now / frame_ms) % count];
}

static const lv_img_dsc_t *petDogFrameForMood(PetDogMood mood, uint32_t now)
{
    switch (mood)
    {
    case PET_DOG_SIT:
        return petFrameAt(pet_dog_sit_frames, sizeof(pet_dog_sit_frames) / sizeof(pet_dog_sit_frames[0]), now, 520);
    case PET_DOG_BARK:
        return petFrameAt(pet_dog_bark_frames, sizeof(pet_dog_bark_frames) / sizeof(pet_dog_bark_frames[0]), now, 125);
    case PET_DOG_RUN:
        return petFrameAt(pet_dog_run_frames, sizeof(pet_dog_run_frames) / sizeof(pet_dog_run_frames[0]), now, 110);
    case PET_DOG_WALK:
        return petFrameAt(pet_dog_walk_frames, sizeof(pet_dog_walk_frames) / sizeof(pet_dog_walk_frames[0]), now, 150);
    case PET_DOG_IDLE:
    default:
        return petFrameAt(pet_dog_idle_frames, sizeof(pet_dog_idle_frames) / sizeof(pet_dog_idle_frames[0]), now, 460);
    }
}

static void maybePlayPetBark(PetDogMood mood, uint32_t now)
{
    if (mood != PET_DOG_BARK)
    {
        last_pet_mood = mood;
        return;
    }

    bool entered_bark = last_pet_mood != PET_DOG_BARK;
    if (hal.pet_bark_sound && (entered_bark || now - last_bark_sound_ms > PET_BARK_COOLDOWN_MS))
    {
        hal.keytone_play = true;
        i2s.playWAV(__pet_bark_wav, __pet_bark_wav_len);
        hal.keytone_play = false;
        last_bark_sound_ms = now;
    }

    last_pet_mood = mood;
}

static const char *moodText(PetDogMood mood, bool waiting)
{
    if (mood == PET_DOG_SIT)
        return "坐下休息";
    if (mood == PET_DOG_BARK)
        return "兴奋汪汪";
    if (mood == PET_DOG_RUN)
        return "高速奔跑";
    if (mood == PET_DOG_WALK)
        return "跟着打字";
    if (waiting)
        return "等你敲键";
    return "轻轻摇尾";
}

static uint32_t moodAccent(const PetTheme &theme, PetDogMood mood)
{
    if (mood == PET_DOG_SIT)
        return theme.sleep;
    if (mood == PET_DOG_BARK || mood == PET_DOG_RUN)
        return theme.excited;
    if (mood == PET_DOG_WALK)
        return theme.focused;
    return theme.accent;
}

static void renderPet()
{
    if (pet_stage == NULL || pet_dog_img == NULL || lbl_pet_status == NULL)
        return;

    uint32_t now = millis();
    uint32_t idle_ms = now - last_key_ms;
    bool sleepy = idle_ms > 90000;
    bool waiting = idle_ms > 20000 && !sleepy;
    PetDogMood mood = currentPetMood(now);
    const PetTheme &theme = petTheme();
    uint32_t accent = moodAccent(theme, mood);
    uint8_t wave = (now / (mood == PET_DOG_RUN ? 95 : 180)) % 4;
    int16_t bob = (mood == PET_DOG_SIT) ? 5 : (wave < 2 ? wave : 3 - wave);

    lv_obj_set_style_bg_color(pet_stage, lv_color_hex(sleepy ? theme.sleep_bg : theme.screen_bg), 0);
    setBox(pet_ground, 20, 118, 280, 31, theme.ground, 14, LV_OPA_COVER);
    setBox(pet_energy_track, 42, 132, 236, 7, theme.track, 4, LV_OPA_COVER);
    setBox(pet_energy_fill, 44, 134, 18 + (int16_t)(pet_energy * 216 / 100), 3, accent, 2, LV_OPA_COVER);
    setBox(pet_shadow, 91, 121, 138, 14, 0x020617, 9, LV_OPA_70);

    const lv_img_dsc_t *frame = petDogFrameForMood(mood, now);
    if (frame != last_pet_frame)
    {
        lv_img_set_src(pet_dog_img, frame);
        last_pet_frame = frame;
    }
    lv_obj_align(pet_dog_img, LV_ALIGN_CENTER, 0, -11 + bob);

    for (uint8_t i = 0; i < 6; i++)
    {
        bool active = pet_energy > (uint16_t)(10 + i * 11);
        showBox(pet_bursts[i], active);
        if (active)
        {
            int16_t x = 38 + i * 45 + (int16_t)((now / 140 + i * 9) % 14);
            int16_t y = 16 + (int16_t)((now / 110 + i * 7) % 56);
            uint8_t size = (mood == PET_DOG_BARK && (i % 2 == 0)) ? 8 : 5;
            setBox(pet_bursts[i], x, y, size, size, i % 2 ? accent : theme.excited, 2, LV_OPA_COVER);
        }
    }

    for (uint8_t i = 0; i < 5; i++)
    {
        bool pressed = (pet_energy > 18) && (((now / 130) + i) % 5 == 0);
        uint32_t key_color = pressed ? accent : theme.key_idle;
        int16_t key_y = pressed ? 145 : 148;
        setBox(pet_keys[i], 82 + i * 34, key_y, 24, 10, key_color, 4, pressed ? LV_OPA_COVER : LV_OPA_80);
    }

    lv_label_set_text(lbl_pet_title, petDisplayName());
    lv_label_set_text_fmt(lbl_pet_status, "%s  热度 %u%%  连击 %u", moodText(mood, waiting), (unsigned int)pet_energy, (unsigned int)combo);

    if (mood == PET_DOG_SIT)
        lv_label_set_text(lbl_pet_hint, "敲几下键叫醒它");
    else if (mood == PET_DOG_BARK)
        lv_label_set_text(lbl_pet_hint, "连击太快，它开始汪汪了");
    else if (mood == PET_DOG_RUN)
        lv_label_set_text(lbl_pet_hint, "打字节奏越快跑得越欢");
    else if (waiting)
        lv_label_set_text(lbl_pet_hint, "等你回来继续");
    else
        lv_label_set_text(lbl_pet_hint, "开源 CC0 像素狗，会跟着打字动");

    setTextColor(lbl_pet_title, sleepy ? theme.sleep : theme.title);
    setTextColor(lbl_pet_status, theme.text);
    setTextColor(lbl_pet_hint, sleepy ? theme.sleep : theme.faint);
}

void AppPet::setup()
{
    hal.LOCKLV();
    lv_obj_clear_flag(_appScreen, LV_OBJ_FLAG_SCROLLABLE);
    const PetTheme &theme = petTheme();
    lv_obj_set_style_bg_color(_appScreen, lv_color_hex(theme.screen_bg), 0);
    lv_obj_set_style_bg_opa(_appScreen, LV_OPA_COVER, 0);
    lv_obj_set_style_text_font(_appScreen, &lv_font_chinese_16, 0);
    lv_obj_set_style_text_color(_appScreen, lv_color_hex(0xE5F2FF), 0);

    lbl_pet_title = lv_label_create(_appScreen);
    lv_label_set_text(lbl_pet_title, petDisplayName());
    lv_obj_set_style_text_font(lbl_pet_title, &lv_font_chinese_16, 0);
    lv_obj_align(lbl_pet_title, LV_ALIGN_TOP_LEFT, 12, 8);

    lbl_pet_status = lv_label_create(_appScreen);
    lv_label_set_text(lbl_pet_status, "轻轻摇尾  热度 0%  连击 0");
    lv_obj_set_style_text_font(lbl_pet_status, &lv_font_chinese_16, 0);
    lv_obj_align(lbl_pet_status, LV_ALIGN_TOP_LEFT, 12, 30);

    pet_stage = createBox(_appScreen, 320, 164, theme.screen_bg, 0);
    lv_obj_set_pos(pet_stage, 0, 46);
    lv_obj_set_style_bg_grad_color(pet_stage, lv_color_hex(theme.grid), 0);
    lv_obj_set_style_bg_grad_dir(pet_stage, LV_GRAD_DIR_VER, 0);

    pet_ground = createBox(pet_stage, 280, 31, theme.ground, 14);
    pet_energy_track = createBox(pet_stage, 236, 7, theme.track, 4);
    pet_energy_fill = createBox(pet_stage, 18, 3, theme.accent, 2);
    pet_shadow = createBox(pet_stage, 138, 14, 0x020617, 9, LV_OPA_70);

    pet_dog_img = lv_img_create(pet_stage);
    lv_img_set_src(pet_dog_img, &pet_dog_idle_0);
    lv_img_set_antialias(pet_dog_img, false);
    lv_obj_align(pet_dog_img, LV_ALIGN_CENTER, 0, -11);

    for (uint8_t i = 0; i < 6; i++)
        pet_bursts[i] = createBox(pet_stage, 5, 5, theme.excited, 2);
    for (uint8_t i = 0; i < 5; i++)
        pet_keys[i] = createBox(pet_stage, 24, 10, theme.key_idle, 4, LV_OPA_80);

    lbl_pet_hint = lv_label_create(_appScreen);
    lv_label_set_text(lbl_pet_hint, "开源 CC0 像素狗，会跟着打字动");
    lv_obj_set_style_text_font(lbl_pet_hint, &lv_font_chinese_16, 0);
    lv_obj_set_width(lbl_pet_hint, 300);
    lv_label_set_long_mode(lbl_pet_hint, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(lbl_pet_hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(lbl_pet_hint, LV_ALIGN_BOTTOM_MID, 0, -8);

    last_key_seq = hal.pet_keypress_seq;
    last_key_ms = millis();
    last_decay_ms = last_key_ms;
    last_bark_sound_ms = 0;
    last_pet_mood = PET_DOG_IDLE;
    last_pet_frame = &pet_dog_idle_0;
    last_render_ms = 0;
    pet_energy = 0;
    combo = 0;
    renderPet();
    hal.UNLOCKLV();
}

void AppPet::loop()
{
    if (!hal.pet_enable)
    {
        xSemaphoreGive(appManagerLite._binary_switchApp);
        return;
    }

    uint32_t now = millis();
    uint32_t seq = hal.pet_keypress_seq;
    if (seq != last_key_seq)
    {
        uint32_t delta = seq - last_key_seq;
        last_key_seq = seq;
        last_key_ms = now;
        combo = addCap(combo, delta, 999);
        uint8_t reactivity = constrain(hal.pet_reactivity, 1, 3);
        pet_energy = addCap(pet_energy, delta * (3 + reactivity * 3) + reactivity, 100);
    }

    if (now - last_decay_ms > 260)
    {
        uint32_t idle_ms = now - last_key_ms;
        uint8_t reactivity = constrain(hal.pet_reactivity, 1, 3);
        uint16_t decay = idle_ms > 90000 ? 5 : (idle_ms > 20000 ? 3 : 1);
        if (reactivity == 1)
            decay++;
        else if (reactivity == 3 && decay > 1)
            decay--;
        if (idle_ms > 900 && pet_energy > 0)
            pet_energy = pet_energy > decay ? pet_energy - decay : 0;
        if (idle_ms > 1500 && combo > 0)
            combo--;
        last_decay_ms = now;
    }

    maybePlayPetBark(currentPetMood(now), now);

    if (now - last_render_ms > 95)
    {
        last_render_ms = now;
        hal.LOCKLV();
        renderPet();
        hal.UNLOCKLV();
    }

    delay(20);
}

void AppPet::destroy()
{
    pet_stage = NULL;
    pet_ground = NULL;
    pet_energy_track = NULL;
    pet_energy_fill = NULL;
    pet_shadow = NULL;
    pet_dog_img = NULL;
    for (uint8_t i = 0; i < 6; i++)
        pet_bursts[i] = NULL;
    for (uint8_t i = 0; i < 5; i++)
        pet_keys[i] = NULL;
    lbl_pet_title = NULL;
    lbl_pet_status = NULL;
    lbl_pet_hint = NULL;
    last_pet_frame = NULL;
}
