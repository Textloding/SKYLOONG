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
    lv_label_set_text_fmt(s_timedisplay_data.p_date_label, "%04d-%02d-%02d", hal.datetime.year, hal.datetime.month, hal.datetime.dayMonth);
}
lv_obj_t *build_time_widget(lv_obj_t *parent)
{
    lv_obj_t *time_widget = lv_obj_create(parent);
    lv_obj_clear_flag(time_widget, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(time_widget, (320 - 8) * (1 - 0.618), (240 - 34) * (1 - 0.618));
    lv_obj_align(time_widget, LV_ALIGN_TOP_RIGHT, -4, 30);
    lv_obj_t *time_label = lv_label_create(time_widget);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_32, 0);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -10);
    lv_obj_t *date_label = lv_label_create(time_widget);
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 20);
    s_timedisplay_data.p_time_label = time_label;
    s_timedisplay_data.p_date_label = date_label;
    s_timedisplay_data.p_time_widget = time_widget;
    lv_timer_create([](lv_timer_t *timer)
                    {
        if(lv_obj_is_valid(s_timedisplay_data.p_time_label) == false || lv_obj_is_valid(s_timedisplay_data.p_date_label) == false)
        {
            lv_timer_del(timer);
            return;
        }
        updateTimeDisplay(); },
                    300, NULL);
    updateTimeDisplay();
    return time_widget;
}

lv_obj_t *build_moon_widget(lv_obj_t *parent)
{
    lv_obj_t *moon_widget = lv_obj_create(parent);
    lv_obj_set_size(moon_widget, (320 - 8) * (1 - 0.618), (240 - 34) * (0.618));
    lv_obj_align(moon_widget, LV_ALIGN_BOTTOM_RIGHT, -4, -4);
    return moon_widget;
}
#include <math.h>
typedef uint16_t (*MyChartGetValueCallback)(); // 获取数据的回调函数，每隔data_interval时间调用一次，非阻塞
class MyChart
{
public:
    lv_obj_t *_chart_widget;
    lv_chart_series_t *_series;
    uint16_t _width;
    uint16_t _height;
    uint16_t _data_buffer[2];      //[0]为上一次数据，[1]为当前数据, 0..1000
    uint16_t _data_weight_current; // 记录距离上一次获取数据以来的图表刷新次数
    uint16_t _data_weight_max;     // 记录插值次数
    uint16_t _data_count;
    uint16_t _data_interval;
    MyChartGetValueCallback _callback;
    lv_timer_t *_timer;
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
    lv_timer_del(_timer);
    lv_obj_del(_chart_widget);
    hal.UNLOCKLV();
}
MyChart chart1;
MyChart chart2;
void AppHome::setup()
{
    hal.LOCKLV();
    build_time_widget(_appScreen);
    build_moon_widget(_appScreen);
    hal.UNLOCKLV();
    chart1.init(_appScreen, (320 - 8) * (0.618), 240 - 34, 1000, 0, []() -> uint16_t
                { return map(hal.APM, 0, 30, 0, 1000); });
    chart1.label_factor = 30 / 1000.0;
    chart1.showLabel(true);
    chart1.showTitle("APS");
    lv_obj_align(chart1._chart_widget, LV_ALIGN_BOTTOM_LEFT, 4, -4);

    chart2.init(
        _appScreen, (320 - 8) * (1 - 0.618), (240 - 34) * (0.618), 200, 500, []() -> uint16_t
        { return rand() % 1000; },
        40);
    chart2.showLabel(false);
    chart2.showTitle("Random");
    lv_obj_align(chart2._chart_widget, LV_ALIGN_BOTTOM_RIGHT, -4, -4);
}

void AppHome::loop()
{
}

void AppHome::destroy()
{
    memset(&s_timedisplay_data, 0, sizeof(s_timedisplay_data));
    chart1.destroy();
    chart2.destroy();
}
