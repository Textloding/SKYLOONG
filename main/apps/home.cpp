#include "A_Config.h"
static struct
{
    lv_obj_t *p_time_widget;
    lv_obj_t *p_time_label;
    lv_obj_t *p_date_label;
} s_timedisplay_data;

void updateTimeDisplay()
{
    hal.getTime();
    static uint8_t last_minute = -1;
    if (hal.datetime.minute == last_minute)
        return;
    if (hal.config_time_12hr)
    {
        if (hal.datetime.hour > 12)
        {
            lv_label_set_text_fmt(s_timedisplay_data.p_time_label, "%02d:%02d", hal.datetime.hour - 12, hal.datetime.minute);
        }
        else
        {
            lv_label_set_text_fmt(s_timedisplay_data.p_time_label, "%02d:%02d", hal.datetime.hour, hal.datetime.minute);
        }
    }
    else
    {
        lv_label_set_text_fmt(s_timedisplay_data.p_time_label, "%02d:%02d", hal.datetime.hour, hal.datetime.minute);
    }
    if (app_settings_save[0].widget == 1)
    {
        lv_obj_set_style_text_font(s_timedisplay_data.p_time_label, &lv_font_montserrat_32, 0);
        lv_obj_align(s_timedisplay_data.p_time_label, LV_ALIGN_CENTER, 0, -10);
        lv_label_set_text_fmt(s_timedisplay_data.p_date_label, "%04d-%02d-%02d", hal.datetime.year, hal.datetime.month, hal.datetime.dayMonth);
        lv_obj_align(s_timedisplay_data.p_date_label, LV_ALIGN_CENTER, 0, 20);
        lv_obj_clear_flag(s_timedisplay_data.p_date_label, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_set_style_text_font(s_timedisplay_data.p_time_label, &lv_font_montserrat_42, 0);
        lv_obj_center(s_timedisplay_data.p_time_label);
        lv_obj_add_flag(s_timedisplay_data.p_date_label, LV_OBJ_FLAG_HIDDEN);
    }
}
lv_obj_t *build_time_widget(lv_obj_t *parent)
{
    lv_obj_t *time_widget = lv_obj_create(parent);
    lv_obj_clear_flag(time_widget, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(time_widget, (320 - 8) * (1 - 0.618), (240 - 34) * (1 - 0.618));
    lv_obj_align(time_widget, LV_ALIGN_TOP_RIGHT, -4, 30);
    lv_obj_t *time_label = lv_label_create(time_widget);
    lv_obj_t *date_label = lv_label_create(time_widget);
    s_timedisplay_data.p_time_label = time_label;
    s_timedisplay_data.p_date_label = date_label;
    s_timedisplay_data.p_time_widget = time_widget;
    if (app_settings_save[0].bg == true)
    {
        lv_obj_set_style_bg_img_src(s_timedisplay_data.p_time_widget, "C:/.data/1.jpg", 0);
    }
    else
    {
        lv_obj_set_style_bg_img_src(s_timedisplay_data.p_time_widget, NULL, 0);
    }
    lv_timer_create([](lv_timer_t *timer)
                    {
        if(lv_obj_is_valid(s_timedisplay_data.p_time_label) == false || lv_obj_is_valid(s_timedisplay_data.p_date_label) == false)
        {
            lv_timer_del(timer);
            return;
        }
        updateTimeDisplay(); },
                    500, NULL);
    updateTimeDisplay();
    return time_widget;
}

#include <moonPhase.h>
moonPhase mp;
lv_img_dsc_t const *moon_imgs[] = {
    &moon_000,
    &moon_010,
    &moon_020,
    &moon_030,
    &moon_040,
    &moon_050,
    &moon_060,
    &moon_070,
    &moon_080,
    &moon_090,
    &moon_100,
    &moon_110,
    &moon_120,
    &moon_130,
    &moon_140,
    &moon_150,
    &moon_160,
    &moon_170,
    &moon_180,
    &moon_190,
    &moon_200,
    &moon_210,
    &moon_220,
    &moon_230,
    &moon_240,
    &moon_250,
    &moon_260,
    &moon_270,
    &moon_280,
    &moon_290,
    &moon_300,
    &moon_310,
    &moon_320,
    &moon_330,
    &moon_340,
    &moon_350,
};
lv_obj_t *build_moon_widget(lv_obj_t *parent)
{
    lv_obj_t *moon_widget = lv_obj_create(parent);
    lv_obj_clear_flag(moon_widget, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(moon_widget, (320 - 8) * (1 - 0.618), (240 - 34) * (0.618));
    lv_obj_align(moon_widget, LV_ALIGN_BOTTOM_RIGHT, -4, -4);
    lv_obj_t *img_moon = lv_img_create(moon_widget);
    lv_obj_align(img_moon, LV_ALIGN_CENTER, 0, 0);
    moonData_t moon;
    tm timenow;
    hal.getTime();
    timenow.tm_year = hal.datetime.year - 1900;
    timenow.tm_mon = hal.datetime.month - 1;
    timenow.tm_mday = hal.datetime.dayMonth;
    timenow.tm_hour = hal.datetime.hour;
    timenow.tm_min = hal.datetime.minute;
    timenow.tm_sec = hal.datetime.second;
    time_t utcnow = mktime(&timenow);
    moon = mp.getPhase(utcnow - i18n::getNTPOffset());

    ESP_LOGI("moon", "moon angle: %d", moon.angle);
    uint16_t angle = moon.angle / 10;
    if (angle >= 36)
        angle = 0;
    lv_img_set_src(img_moon, moon_imgs[angle]);
    return moon_widget;
}

lv_obj_t *build_img_right_widget(lv_obj_t *parent)
{
    lv_obj_t *img_widget = lv_obj_create(parent);
    lv_obj_set_size(img_widget, (320 - 8) * (1 - 0.618), (240 - 34) * (0.618));
    lv_obj_align(img_widget, LV_ALIGN_BOTTOM_RIGHT, -4, -4);
    lv_obj_set_style_bg_img_src(img_widget, "C:/.data/2.jpg", 0);
    return img_widget;
}

lv_obj_t *build_img_left_widget(lv_obj_t *parent)
{
    lv_obj_t *img_widget = lv_obj_create(parent);
    lv_obj_set_size(img_widget, (320 - 8) * (0.618), (240 - 34));
    lv_obj_align(img_widget, LV_ALIGN_BOTTOM_LEFT, 4, -4);
    lv_obj_set_style_bg_img_src(img_widget, "C:/.data/3.jpg", 0);
    return img_widget;
}
#include <math.h>
typedef uint16_t (*MyChartGetValueCallback)(); // 获取数据的回调函数，每隔data_interval时间调用一次，非阻塞
class MyChart
{
public:
    lv_obj_t *_chart_widget = NULL;
    lv_chart_series_t *_series = NULL;
    uint16_t _width;
    uint16_t _height;
    uint16_t _data_buffer[2];      //[0]为上一次数据，[1]为当前数据, 0..1000
    uint16_t _data_weight_current; // 记录距离上一次获取数据以来的图表刷新次数
    uint16_t _data_weight_max;     // 记录插值次数
    uint16_t _data_count;
    uint16_t _data_interval;
    MyChartGetValueCallback _callback;
    lv_timer_t *_timer = NULL;
    lv_obj_t *label = NULL;
    float label_factor = 1.0;
    lv_obj_t *lbl_title = NULL;
    uint16_t _update_interval = 20;
    void init(lv_obj_t *parent, uint16_t width, uint16_t height, uint16_t data_interval, uint16_t default_value, MyChartGetValueCallback callback, uint16_t update_interval = 20);
    void destroy();
    void showLabel(bool show)
    {
        hal.LOCKLV();
        if (show)
        {
            if (label)
                goto end;
            label = lv_label_create(_chart_widget);
            lv_obj_set_style_bg_color(label, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
            lv_obj_set_style_bg_opa(label, LV_OPA_30, 0);
            lv_obj_set_style_radius(label, 3, 0);
            lv_obj_align(label, LV_ALIGN_TOP_RIGHT, -5, 0);
        }
        else
        {
            if (!label)
                goto end;
            lv_obj_del(label);
            label = NULL;
        }
    end:
        hal.UNLOCKLV();
    }
    void showTitle(const char *title)
    {
        hal.LOCKLV();
        if (title)
        {
            if (lbl_title)
            {
                lv_label_set_text(lbl_title, title);
                goto end;
            }
            lbl_title = lv_label_create(_chart_widget);

            lv_obj_set_style_text_font(lbl_title, &lv_font_chinese_16, 0);
            lv_label_set_text(lbl_title, title);
            lv_obj_set_style_bg_color(lbl_title, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
            lv_obj_set_style_bg_opa(lbl_title, LV_OPA_30, 0);
            lv_obj_set_style_radius(lbl_title, 3, 0);
            lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 0);
        }
        else
        {
            if (lbl_title)
            {
                lv_obj_del(lbl_title);
                lbl_title = NULL;
            }
        }
    end:
        hal.UNLOCKLV();
    }
    uint16_t cosineInterpolation()
    {
        float result = 0.0;
        float point = (float)_data_weight_current / (float)_data_weight_max;

        // 计算权重
        float weight = (1 - cos((point)*M_PI)) / 2;

        // 执行余弦插值计算
        result = (1 - weight) * _data_buffer[0] + weight * _data_buffer[1];

        return result;
    }
};
void MyChart::init(lv_obj_t *parent, uint16_t width, uint16_t height, uint16_t data_interval, uint16_t default_value, MyChartGetValueCallback callback, uint16_t update_interval)
{
    hal.LOCKLV();
    _chart_widget = lv_chart_create(parent);
    lv_obj_set_size(_chart_widget, width, height);
    lv_chart_set_update_mode(_chart_widget, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_point_count(_chart_widget, width - 2);
    lv_obj_set_style_line_width(_chart_widget, 1, LV_PART_ITEMS);
    lv_chart_set_range(_chart_widget, LV_CHART_AXIS_PRIMARY_Y, 0, 1000);
    lv_chart_set_type(_chart_widget, LV_CHART_TYPE_LINE);
    _series = lv_chart_add_series(_chart_widget, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
    lv_obj_set_style_opa(_chart_widget, LV_OPA_0, LV_PART_INDICATOR);
    lv_obj_set_style_pad_all(_chart_widget, 0, 0);
    lv_chart_set_all_value(_chart_widget, _series, default_value);
    lv_obj_clear_flag(_chart_widget, LV_OBJ_FLAG_SCROLLABLE);
    _timer = lv_timer_create([](lv_timer_t *timer)
                             {
        MyChart *chart_ptr = (MyChart *)timer->user_data;
        if(chart_ptr->_data_weight_max == chart_ptr->_data_weight_current)
        {
            chart_ptr->_data_weight_current = 0;
            if(chart_ptr->_callback != NULL)
            {
                chart_ptr->_data_buffer[0] = chart_ptr->_data_buffer[1];
                chart_ptr->_data_buffer[1] = chart_ptr->_callback();
            }
        }
        float next = chart_ptr->cosineInterpolation();
        lv_chart_set_next_value(chart_ptr->_chart_widget, chart_ptr->_series, next);
        if(chart_ptr->label != NULL)
        {
            lv_label_set_text_fmt(chart_ptr->label, "%d", (int)(next * chart_ptr->label_factor));
            float pos = (1000.0 - next) * chart_ptr->_height / 1000.0;
            if(pos < 2)
                pos = 2;
            if(pos + lv_obj_get_height(chart_ptr->label) >= chart_ptr->_height - 5)
                pos -= lv_obj_get_height(chart_ptr->label) + 5;
            lv_obj_set_y(chart_ptr->label, (uint16_t)(pos));
        }
        chart_ptr->_data_weight_current++; },
                             _update_interval, this);
    hal.UNLOCKLV();
    _width = width;
    _height = height;
    _data_count = (width - 2) * _update_interval / data_interval;
    _data_interval = data_interval;
    _callback = callback;
    _data_weight_current = 0;
    _data_weight_max = (width - 2) / _data_count;
    _data_buffer[0] = default_value;
    _data_buffer[1] = default_value;
    label = NULL;
    lbl_title = NULL;
    label_factor = 1.0;
}
void MyChart::destroy()
{
    hal.LOCKLV();
    if (_timer)
        lv_timer_del(_timer);
    if (_chart_widget)
        lv_obj_del(_chart_widget);
    _timer = NULL;
    _chart_widget = NULL;
    label = NULL;
    hal.UNLOCKLV();
}
MyChart chart1;
MyChart chart2;
#include <esp_websocket_client.h>
struct tcp_client_data
{
    float cpu_pct;
    float mem_pct;
};
static bool tcp_connected;
static struct tcp_client_data s_tcp_client_data;
static WiFiClient tcpClient; // 用于获取CPU或内存数据
static esp_websocket_client_handle_t client1 = NULL;
static esp_websocket_client_handle_t client2 = NULL;
uint16_t websocket_data1 = 0; // 用于暂存外部数据源数据
uint16_t websocket_data2 = 0;
static bool i_started_wifi = false;
static bool page_has_remote = false;
static bool tcp_started = false;
static uint16_t handleTCPClient(uint8_t data_source)
{
    if (data_source == 0)
        return (uint16_t)(s_tcp_client_data.cpu_pct * 1000);
    else
        return (uint16_t)(s_tcp_client_data.mem_pct * 1000);
}
static void handleWebSocket(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    if (event_id == WEBSOCKET_EVENT_DATA)
    {
        ESP_LOGI("websocket", "Received data: %s", data->data_ptr);
        *(uint16_t *)data->user_context = atoi(data->data_ptr);
    }
}
void AppHome::setup()
{
    page_has_remote = false;
    tcp_started = false;
    hal.LOCKLV();
    build_time_widget(_appScreen);
    hal.UNLOCKLV();
    if (app_settings_save[1].widget == 0)
    {
        if (app_settings_save[1].data == 2)
        {
            // APS
            chart1.init(_appScreen, (320 - 8) * (1 - 0.618), (240 - 34) * (0.618), 1000, 0, []() -> uint16_t
                        { return map(hal.APM, 0, 30, 0, 1000); });
            chart1.label_factor = 30 / 1000.0;
        }
        else if (app_settings_save[1].data == 4)
        {
            // 随机数
            chart1.init(_appScreen, (320 - 8) * (1 - 0.618), (240 - 34) * (0.618), 500, 0, []() -> uint16_t
                        { return rand() % 1000; });
            chart1.label_factor = 0.1;
        }
        else
        {
            page_has_remote = true;
            if (app_settings_save[1].data == 0 || app_settings_save[1].data == 1)
            {

                chart1.init(_appScreen, (320 - 8) * (1 - 0.618), (240 - 34) * (0.618), 500, 0, []() -> uint16_t
                            { return handleTCPClient(app_settings_save[1].data); });
                chart1.label_factor = 0.1;
                tcp_started = true;
            }
            else
            {
                chart1.init(
                    _appScreen, (320 - 8) * (1 - 0.618), (240 - 34) * (0.618), app_settings_save[1].ext_interval, 0, []() -> uint16_t
                    { return websocket_data1; },
                    app_settings_save[1].ext_interpolation);
                if (app_settings_save[1].ext_url[0] != 0)
                {
                    esp_websocket_client_config_t websocket_cfg;
                    memset(&websocket_cfg, 0, sizeof(websocket_cfg));
                    websocket_cfg.uri = app_settings_save[1].ext_url;
                    client1 = esp_websocket_client_init(&websocket_cfg);
                    if (client1)
                        esp_websocket_register_events(client1, WEBSOCKET_EVENT_DATA, handleWebSocket, &websocket_data1);
                }
            }
        }
        chart1.showLabel(app_settings_save[1].showindicator);
        if (app_settings_save[1].showlbl && app_settings_save[1].lbl[0] != 0)
            chart1.showTitle(app_settings_save[1].lbl);
        lv_obj_align(chart1._chart_widget, LV_ALIGN_BOTTOM_RIGHT, -4, -4);
    }
    else if (app_settings_save[1].widget == 1)
    {
        hal.LOCKLV();
        build_moon_widget(_appScreen);
        hal.UNLOCKLV();
    }
    else
    {
        hal.LOCKLV();
        build_img_right_widget(_appScreen);
        hal.UNLOCKLV();
    }
    if (app_settings_save[2].widget == 0)
    {
        if (app_settings_save[2].data == 2)
        {
            // APS
            chart2.init(_appScreen, (320 - 8) * (0.618), 240 - 34, 1000, 0, []() -> uint16_t
                        { return map(hal.APM, 0, 30, 0, 1000); });
            chart2.label_factor = 30 / 1000.0;
        }
        else if (app_settings_save[2].data == 4)
        {
            // 随机数
            chart2.init(_appScreen, (320 - 8) * (0.618), 240 - 34, 500, 0, []() -> uint16_t
                        { return rand() % 1000; });
            chart2.label_factor = 0.1;
        }
        else
        {
            page_has_remote = true;
            if (app_settings_save[2].data == 0 || app_settings_save[2].data == 1)
            {

                chart2.init(_appScreen, (320 - 8) * (0.618), 240 - 34, 500, 0, []() -> uint16_t
                            { return handleTCPClient(app_settings_save[2].data); });
                chart2.label_factor = 0.1;
                tcp_started = true;
            }
            else
            {
                chart2.init(
                    _appScreen, (320 - 8) * (0.618), 240 - 34, app_settings_save[2].ext_interval, 0, []() -> uint16_t
                    { return websocket_data2; },
                    app_settings_save[2].ext_interpolation);
                if (app_settings_save[2].ext_url[0] != 0)
                {
                    esp_websocket_client_config_t websocket_cfg;
                    memset(&websocket_cfg, 0, sizeof(websocket_cfg));
                    websocket_cfg.uri = app_settings_save[2].ext_url;
                    client2 = esp_websocket_client_init(&websocket_cfg);
                    if (client2)
                        esp_websocket_register_events(client2, WEBSOCKET_EVENT_DATA, handleWebSocket, &websocket_data2);
                }
            }
        }
        chart2.showLabel(app_settings_save[2].showindicator);
        if (app_settings_save[2].showlbl && app_settings_save[2].lbl[0] != 0)
            chart2.showTitle(app_settings_save[2].lbl);
        lv_obj_align(chart2._chart_widget, LV_ALIGN_BOTTOM_LEFT, 4, -4);
    }
    else
    {
        hal.LOCKLV();
        build_img_left_widget(_appScreen);
        hal.UNLOCKLV();
    }

    if (page_has_remote)
    {
        if (WiFi.getMode() != WIFI_STA)
        {
            if (WiFiMgr.requireWiFi())
            {
                i_started_wifi = true;
                // 连接服务器
                if (tcp_started)
                {
                    IPAddress remote;
                    remote.fromString(app_settings_remote_ip);
                    if (tcpClient.connect(remote, app_settings_remote_port))
                    {
                        tcp_connected = true;
                    }
                    else
                    {
                        tcpClient.stop();
                        GUI::toast(_tr(I18N_ID_TCP_FAILED));
                        tcp_connected = false;
                        tcp_started = false;
                    }
                }
                if (client1)
                {
                    esp_websocket_client_start(client1);
                }
                if (client2)
                {
                    esp_websocket_client_start(client2);
                }
            }
            else
            {
                page_has_remote = false;
                GUI::toast(_tr(I18N_ID_OFFLINE_MODE));
            }
        }
    }
}

void AppHome::loop()
{
    if (page_has_remote)
    {
        if (WiFi.isConnected() == false)
        {
            if (client1)
                esp_websocket_client_stop(client1);
            if (client2)
                esp_websocket_client_stop(client2);
            tcpClient.stop();
            if (WiFiMgr.requireWiFi())
            {
                if (client1)
                    esp_websocket_client_start(client1);
                if (client2)
                    esp_websocket_client_start(client2);
            }
            else
            {
                if (client1)
                    esp_websocket_client_destroy(client1);
                if (client2)
                    esp_websocket_client_destroy(client2);
                tcp_connected = tcp_started = false;
                client1 = NULL;
                client2 = NULL;
                page_has_remote = false;
                GUI::toast(_tr(I18N_ID_OFFLINE_MODE));
            }
        }
        else
        {
            if (tcp_connected)
            {
                if (tcpClient.available() >= sizeof(s_tcp_client_data))
                {
                    tcpClient.readBytes((char *)&s_tcp_client_data, sizeof(s_tcp_client_data));
                    tcpClient.write((uint8_t)0x00);
                }
                if (tcpClient.connected() == false)
                {
                    tcp_connected = false;
                    GUI::toast(_tr(I18N_ID_TCP_DISCONNECTED));
                }
            }
            else if (tcp_started)
            {
                IPAddress remote;
                remote.fromString(app_settings_remote_ip);
                if (tcpClient.connect(remote, app_settings_remote_port))
                {
                    tcp_connected = true;
                }
                else
                {
                    tcpClient.stop();
                    GUI::toast(_tr(I18N_ID_TCP_FAILED));
                    tcp_connected = false;
                    tcp_started = false;
                }
            }
        }
    }
    delay(100);
}

void AppHome::destroy()
{
    memset(&s_timedisplay_data, 0, sizeof(s_timedisplay_data));
    chart1.destroy();
    chart2.destroy();
    if (client1)
    {
        esp_websocket_client_stop(client1);
        esp_websocket_client_destroy(client1);
        client1 = NULL;
    }
    if (client2)
    {
        esp_websocket_client_stop(client2);
        esp_websocket_client_destroy(client2);
        client2 = NULL;
    }
    tcpClient.stop();
    if (i_started_wifi)
    {
        WiFiMgr.disconnect();
        i_started_wifi = false;
    }
}
