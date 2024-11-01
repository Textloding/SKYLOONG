#pragma once
#include "lvgl.h"
#include "Arduino.h"
#include <stack>
class BaseApp
{
public:
    bool _addToSwitchApp = true; // 是否将当前App加入到切换列表
    uint32_t appid = 0;
    lv_obj_t *_appScreen = NULL;
    virtual void setup() = 0;
    virtual void loop() = 0;
    virtual void destroy() = 0; // 销毁当前App，注意应该在此处释放LVGL定时器
    virtual void stop(){};      // 停止loop中正在运行的循环
};

extern uint32_t RTC_DATA_ATTR last_appid;
class AppManagerLite
{
public:
    BaseApp *currentApp = NULL;
    BaseApp *lastAppBeforeSetting = NULL;
    BaseApp *appHome = NULL;
    BaseApp *appAPS = NULL;
    BaseApp *appGIF = NULL;
    BaseApp *appJPG = NULL;
    BaseApp *appWeather = NULL;
    BaseApp *appSysinfo = NULL;
    BaseApp *appSettings = NULL;
    BaseApp *appQRCode = NULL;
    BaseApp *getAppByName(const uint32_t appid);
    void init(const uint32_t lastAppid);
    void switchApp(BaseApp *app);
    void switchApp(const uint32_t appid)
    {
        switchApp(getAppByName(appid));
    }
    void switchNext() {
        BaseApp *nextApp = NULL;
        if (currentApp != NULL) {
            switch (currentApp->appid) {
                case 1:
                    if (hal.aps_enable) {
                        nextApp = appAPS;
                        break;   
                    }
                case 2:
                    if (hal.gif_enable) {
                        nextApp = appGIF;
                        break;   
                    }
                case 3:
                    if (hal.jpg_enable) {
                        nextApp = appJPG;
                        break;   
                    }
                case 4:
                    if (hal.weather_enable) {
                        nextApp = appWeather;
                        break;   
                    }
                case 5:
                    if (hal.sysinfo_enable) {
                        nextApp = appSysinfo;
                        break;   
                    }
                case 6:
                    nextApp = appQRCode;
                    break;
                case 50:
                case 100:
                    nextApp = appHome;
                    break;
            }
        }

        switchApp(nextApp);
    }
    void switchSetting()
    {
        if (currentApp->appid == 50)
            return;
        lastAppBeforeSetting = currentApp;
        switchApp(50);
    }
    void exitSetting()
    {
        if (currentApp->appid == 50)
            switchApp(lastAppBeforeSetting);
    }
    void switchQRCode()
    {
        if (currentApp->appid == 100)
            return;
        switchApp(100);
    }
    void exitQRCode()
    {
        if (currentApp->appid == 100) {
            switchApp(1);
        }
    }
    void loop();
    SemaphoreHandle_t _mutex = NULL;
    SemaphoreHandle_t _binary_switchApp = NULL;
};
extern AppManagerLite appManagerLite;
