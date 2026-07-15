#include <A_Config.h>
#include "ESP_I2S.h"
#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"
#include "GUI.h"
extern I2SClass i2s;
static const int Table_fv1[256] = {-180, -179, -177, -176, -174, -173, -172, -170, -169, -167, -166, -165, -163, -162, -160, -159, -158, -156, -155, -153, -152, -151, -149, -148, -146, -145, -144, -142, -141, -139, -138, -137, -135, -134, -132, -131, -130, -128, -127, -125, -124, -123, -121, -120, -118, -117, -115, -114, -113, -111, -110, -108, -107, -106, -104, -103, -101, -100, -99, -97, -96, -94, -93, -92, -90, -89, -87, -86, -85, -83, -82, -80, -79, -78, -76, -75, -73, -72, -71, -69, -68, -66, -65, -64, -62, -61, -59, -58, -57, -55, -54, -52, -51, -50, -48, -47, -45, -44, -43, -41, -40, -38, -37, -36, -34, -33, -31, -30, -29, -27, -26, -24, -23, -22, -20, -19, -17, -16, -15, -13, -12, -10, -9, -8, -6, -5, -3, -2, 0, 1, 2, 4, 5, 7, 8, 9, 11, 12, 14, 15, 16, 18, 19, 21, 22, 23, 25, 26, 28, 29, 30, 32, 33, 35, 36, 37, 39, 40, 42, 43, 44, 46, 47, 49, 50, 51, 53, 54, 56, 57, 58, 60, 61, 63, 64, 65, 67, 68, 70, 71, 72, 74, 75, 77, 78, 79, 81, 82, 84, 85, 86, 88, 89, 91, 92, 93, 95, 96, 98, 99, 100, 102, 103, 105, 106, 107, 109, 110, 112, 113, 114, 116, 117, 119, 120, 122, 123, 124, 126, 127, 129, 130, 131, 133, 134, 136, 137, 138, 140, 141, 143, 144, 145, 147, 148, 150, 151, 152, 154, 155, 157, 158, 159, 161, 162, 164, 165, 166, 168, 169, 171, 172, 173, 175, 176, 178};
static const int Table_fv2[256] = {-92, -91, -91, -90, -89, -88, -88, -87, -86, -86, -85, -84, -83, -83, -82, -81, -81, -80, -79, -78, -78, -77, -76, -76, -75, -74, -73, -73, -72, -71, -71, -70, -69, -68, -68, -67, -66, -66, -65, -64, -63, -63, -62, -61, -61, -60, -59, -58, -58, -57, -56, -56, -55, -54, -53, -53, -52, -51, -51, -50, -49, -48, -48, -47, -46, -46, -45, -44, -43, -43, -42, -41, -41, -40, -39, -38, -38, -37, -36, -36, -35, -34, -33, -33, -32, -31, -31, -30, -29, -28, -28, -27, -26, -26, -25, -24, -23, -23, -22, -21, -21, -20, -19, -18, -18, -17, -16, -16, -15, -14, -13, -13, -12, -11, -11, -10, -9, -8, -8, -7, -6, -6, -5, -4, -3, -3, -2, -1, 0, 0, 1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 8, 9, 10, 10, 11, 12, 12, 13, 14, 15, 15, 16, 17, 17, 18, 19, 20, 20, 21, 22, 22, 23, 24, 25, 25, 26, 27, 27, 28, 29, 30, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 37, 38, 39, 40, 40, 41, 42, 42, 43, 44, 45, 45, 46, 47, 47, 48, 49, 50, 50, 51, 52, 52, 53, 54, 55, 55, 56, 57, 57, 58, 59, 60, 60, 61, 62, 62, 63, 64, 65, 65, 66, 67, 67, 68, 69, 70, 70, 71, 72, 72, 73, 74, 75, 75, 76, 77, 77, 78, 79, 80, 80, 81, 82, 82, 83, 84, 85, 85, 86, 87, 87, 88, 89, 90, 90};
static const int Table_fu1[256] = {-44, -44, -44, -43, -43, -43, -42, -42, -42, -41, -41, -41, -40, -40, -40, -39, -39, -39, -38, -38, -38, -37, -37, -37, -36, -36, -36, -35, -35, -35, -34, -34, -33, -33, -33, -32, -32, -32, -31, -31, -31, -30, -30, -30, -29, -29, -29, -28, -28, -28, -27, -27, -27, -26, -26, -26, -25, -25, -25, -24, -24, -24, -23, -23, -22, -22, -22, -21, -21, -21, -20, -20, -20, -19, -19, -19, -18, -18, -18, -17, -17, -17, -16, -16, -16, -15, -15, -15, -14, -14, -14, -13, -13, -13, -12, -12, -11, -11, -11, -10, -10, -10, -9, -9, -9, -8, -8, -8, -7, -7, -7, -6, -6, -6, -5, -5, -5, -4, -4, -4, -3, -3, -3, -2, -2, -2, -1, -1, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 22, 22, 22, 23, 23, 23, 24, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 40, 40, 40, 41, 41, 41, 42, 42, 42, 43, 43};
static const int Table_fu2[256] = {-227, -226, -224, -222, -220, -219, -217, -215, -213, -212, -210, -208, -206, -204, -203, -201, -199, -197, -196, -194, -192, -190, -188, -187, -185, -183, -181, -180, -178, -176, -174, -173, -171, -169, -167, -165, -164, -162, -160, -158, -157, -155, -153, -151, -149, -148, -146, -144, -142, -141, -139, -137, -135, -134, -132, -130, -128, -126, -125, -123, -121, -119, -118, -116, -114, -112, -110, -109, -107, -105, -103, -102, -100, -98, -96, -94, -93, -91, -89, -87, -86, -84, -82, -80, -79, -77, -75, -73, -71, -70, -68, -66, -64, -63, -61, -59, -57, -55, -54, -52, -50, -48, -47, -45, -43, -41, -40, -38, -36, -34, -32, -31, -29, -27, -25, -24, -22, -20, -18, -16, -15, -13, -11, -9, -8, -6, -4, -2, 0, 1, 3, 5, 7, 8, 10, 12, 14, 15, 17, 19, 21, 23, 24, 26, 28, 30, 31, 33, 35, 37, 39, 40, 42, 44, 46, 47, 49, 51, 53, 54, 56, 58, 60, 62, 63, 65, 67, 69, 70, 72, 74, 76, 78, 79, 81, 83, 85, 86, 88, 90, 92, 93, 95, 97, 99, 101, 102, 104, 106, 108, 109, 111, 113, 115, 117, 118, 120, 122, 124, 125, 127, 129, 131, 133, 134, 136, 138, 140, 141, 143, 145, 147, 148, 150, 152, 154, 156, 157, 159, 161, 163, 164, 166, 168, 170, 172, 173, 175, 177, 179, 180, 182, 184, 186, 187, 189, 191, 193, 195, 196, 198, 200, 202, 203, 205, 207, 209, 211, 212, 214, 216, 218, 219, 221, 223, 225};
static __always_inline uint8_t clamp(int n)
{
    if (n > 255)
    {
        n = 255;
    }
    else if (n & 0x80000000)
    {
        n = 0;
    }
    return n;
}

void ConvertYUV420SPToBGR(const plm_frame_t *frame, unsigned char *Dst)
{
    if (frame == NULL || frame->width < 1 || frame->height < 1 || Dst == NULL)
        return;

    int rdif, invgdif, bdif;
    uint16_t rgb565;
    for (unsigned int i = 0; i < frame->height; i++)
    {
        const uint8_t *yRow = frame->y.data + i * frame->y.width;
        const uint8_t *crRow = frame->cr.data + (i / 2) * frame->cr.width;
        const uint8_t *cbRow = frame->cb.data + (i / 2) * frame->cb.width;
        for (unsigned int j = 0; j < frame->width; j++)
        {
            const uint8_t y = yRow[j];
            const uint8_t cr = crRow[j / 2];
            const uint8_t cb = cbRow[j / 2];

            rdif = Table_fv1[cr];
            invgdif = Table_fu1[cb] + Table_fv2[cr];
            bdif = Table_fu2[cb];
            rgb565 = (clamp(y + rdif) >> 3) << 11;
            rgb565 |= (clamp(y - invgdif) >> 2) << 5;
            rgb565 |= (clamp(y + bdif) >> 3);
            *(Dst++) = (rgb565 >> 8) & 0xFF;
            *(Dst++) = rgb565 & 0xFF;
        }
    }
}

uint8_t *rgb_buffer = NULL;
static bool video_background_cleared = false;
static int16_t audio_pcm_buffer[PLM_AUDIO_SAMPLES_PER_FRAME * 2];

static int16_t floatToPcm16(float sample)
{
    if (sample > 1.0f)
        sample = 1.0f;
    else if (sample < -1.0f)
        sample = -1.0f;
    return (int16_t)(sample * 32767.0f);
}

void my_audio_callback(plm_t *plm, plm_samples_t *samples, void *user)
{
    if (samples == NULL || samples->count == 0 || !hal.config_video_audio || !hal.audio_ready.load())
        return;

    const unsigned int sample_count = samples->count * 2;
    for (unsigned int i = 0; i < sample_count; ++i)
    {
        audio_pcm_buffer[i] = floatToPcm16(samples->interleaved[i]);
    }

    i2s.write((uint8_t *)audio_pcm_buffer, sample_count * sizeof(int16_t));
}

void my_video_callback(plm_t *plm, plm_frame_t *frame, void *user)
{
    if (frame == NULL || frame->width == 0 || frame->height == 0)
        return;

    if (frame->width > screenWidth || frame->height > screenHeight)
    {
        ESP_LOGE("MPEG", "Unsupported frame size: %ux%u", frame->width, frame->height);
        return;
    }

    if (!video_background_cleared)
    {
        tft.fillScreen(TFT_BLACK);
        video_background_cleared = true;
    }

    ConvertYUV420SPToBGR(frame, rgb_buffer);
    tft.startWrite();
    tft.setAddrWindow((screenWidth - frame->width) / 2, (screenHeight - frame->height) / 2, frame->width, frame->height);
    tft.pushColors(rgb_buffer, frame->width * frame->height * 2);
    tft.endWrite();
}
extern bool get_gif_vid_stop();

static bool playRawMpegVideo(plm_video_t *video)
{
    if (video == NULL)
        return false;

    if (!plm_video_has_header(video))
    {
        ESP_LOGE("MPEG", "Unsupported MPEG stream");
        return false;
    }

    const int width = plm_video_get_width(video);
    const int height = plm_video_get_height(video);
    if (width <= 0 || height <= 0 || width > screenWidth || height > screenHeight)
    {
        ESP_LOGE("MPEG", "Unsupported raw video size: %dx%d", width, height);
        return true;
    }

    float frame_rate = plm_video_get_framerate(video);
    if (frame_rate < 1.0f || frame_rate > 30.0f)
        frame_rate = 15.0f;

    const uint32_t frame_delay = (uint32_t)(1000.0f / frame_rate);
    while (true)
    {
        plm_frame_t *frame = plm_video_decode(video);
        if (frame == NULL)
        {
            if (videoPlayer.video_loop)
            {
                plm_video_rewind(video);
                continue;
            }
            break;
        }

        my_video_callback(NULL, frame, NULL);
        delay(frame_delay);

        if (get_gif_vid_stop())
            break;

        if (plm_video_has_ended(video) && !videoPlayer.video_loop)
            break;
    }
    return true;
}

static bool playMpeg(plm_t *plm, bool allow_audio)
{
    if (plm == NULL)
        return false;

    if (!allow_audio)
        plm_set_audio_enabled(plm, false);
    plm_set_loop(plm, videoPlayer.video_loop);
    hal.setBrightness(hal._brightness);

    if (plm_get_num_video_streams(plm) <= 0)
    {
        ESP_LOGE("MPEG", "Unsupported MPEG stream");
        return false;
    }

    const int width = plm_get_width(plm);
    const int height = plm_get_height(plm);
    if (width <= 0 || height <= 0)
    {
        ESP_LOGE("MPEG", "Invalid MPEG stream");
        return true;
    }
    if (width > screenWidth || height > screenHeight)
    {
        ESP_LOGE("MPEG", "Unsupported video size: %dx%d", width, height);
        return true;
    }

    float frame_rate = plm_get_framerate(plm);
    if (frame_rate < 1.0f || frame_rate > 30.0f)
        frame_rate = 15.0f;

    bool audio_enabled = false;
    if (allow_audio && hal.config_video_audio && hal.audio_ready.load() && plm_get_num_audio_streams(plm) > 0)
    {
        const int sample_rate = plm_get_samplerate(plm);
        if (sample_rate == AUDIO_SAMPLE_RATE && hal.audio_begin_playback())
        {
            audio_enabled = hal.audio_configure_tx(sample_rate,
                                                   I2S_DATA_BIT_WIDTH_16BIT,
                                                   I2S_SLOT_MODE_STEREO);
            if (!audio_enabled)
                hal.audio_end_playback();
        }
        else
        {
            ESP_LOGW("MPEG", "Video audio disabled, unsupported sample rate: %d", sample_rate);
        }
    }

    plm_set_video_decode_callback(plm, my_video_callback, NULL);
    if (audio_enabled)
    {
        plm_set_audio_decode_callback(plm, my_audio_callback, NULL);
        plm_set_audio_lead_time(plm, 0.08f);
    }
    else
    {
        plm_set_audio_enabled(plm, false);
    }

    const uint32_t fallback_frame_delay = (uint32_t)(1000.0f / frame_rate);
    uint32_t last_decode_millis = millis();

    while (!plm_has_ended(plm))
    {
        uint32_t now = millis();
        uint32_t elapsed_ms = now - last_decode_millis;
        last_decode_millis = now;
        if (elapsed_ms == 0)
            elapsed_ms = 1;
        if (!audio_enabled && elapsed_ms > fallback_frame_delay)
            elapsed_ms = fallback_frame_delay;

        plm_decode(plm, elapsed_ms / 1000.0f);
        delay(1);

        if (get_gif_vid_stop())
            break;
    }
    if (audio_enabled)
        hal.audio_end_playback();
    return true;
}

void VideoPlayer::play(FILE *f)
{
    if (f == NULL)
        return;

    hal.LOCKLV();
    if (rgb_buffer == NULL)
        rgb_buffer = (uint8_t *)ps_malloc(screenWidth * screenHeight * 2);
    if (rgb_buffer == NULL)
    {
        ESP_LOGE("MPEG", "Frame buffer allocation failed; skipping video");
        hal.UNLOCKLV();
        return;
    }
    memset(rgb_buffer, 0, screenWidth * screenHeight * 2);
    video_background_cleared = false;
    plm_buffer_t *buffer = plm_buffer_create_with_file(f, false);
    if (buffer == NULL)
    {
        ESP_LOGE("MPEG", "Stream buffer allocation failed; skipping video");
        hal.UNLOCKLV();
        return;
    }
    plm_t *plm = plm_create_with_buffer(buffer, true);
    if (plm == NULL)
    {
        ESP_LOGE("MPEG", "Decoder allocation failed; skipping video");
        hal.UNLOCKLV();
        return;
    }
    bool handled = playMpeg(plm, true);

    plm_destroy(plm);
    if (!handled)
    {
        fseek(f, 0, SEEK_SET);
        plm_buffer_t *raw_buffer = plm_buffer_create_with_file(f, false);
        if (raw_buffer != NULL)
        {
            plm_video_t *video = plm_video_create_with_buffer(raw_buffer, true);
            if (video != NULL)
            {
                playRawMpegVideo(video);
                plm_video_destroy(video);
            }
        }
    }
    hal.UNLOCKLV();
}

void VideoPlayer::playBuffer(const uint8_t *video_buffer, uint32_t buffer_size)
{
    if (video_buffer == NULL || buffer_size == 0)
        return;

    videoPlayer.video_loop = false;
    hal.LOCKLV();
    const bool owns_rgb_buffer = rgb_buffer == NULL;
    if (owns_rgb_buffer)
        rgb_buffer = (uint8_t *)ps_malloc(screenWidth * screenHeight * 2);
    if (rgb_buffer == NULL)
    {
        ESP_LOGE("MPEG", "Boot frame buffer allocation failed; skipping animation");
        hal.UNLOCKLV();
        return;
    }
    memset(rgb_buffer, 0, screenWidth * screenHeight * 2);
    video_background_cleared = false;
    plm_buffer_t *buffer = plm_buffer_create_with_memory((uint8_t *)video_buffer, buffer_size, false);
    plm_t *plm = buffer == NULL ? NULL : plm_create_with_buffer(buffer, true);
    if (plm != NULL)
    {
        playMpeg(plm, false);
        plm_destroy(plm);
    }
    else
    {
        ESP_LOGE("MPEG", "Boot decoder allocation failed; skipping animation");
    }

    if (owns_rgb_buffer)
    {
        free(rgb_buffer);
        rgb_buffer = NULL;
    }
    hal.UNLOCKLV();
}

VideoPlayer videoPlayer;
