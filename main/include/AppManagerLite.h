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
    bool requireSettings = false;
    bool hideStatusBar = false;
    virtual void setup() = 0;
    virtual void loop() = 0;
    virtual void destroy() = 0; // 销毁当前App，注意应该在此处释放LVGL定时器
    virtual void stop(){};            // 停止loop中正在运行的循环
};
extern BaseApp *appList[50];
extern int appListLen;
extern uint32_t RTC_DATA_ATTR last_appid;
class AppManagerLite
{
public:
    BaseApp *currentApp = NULL;
    BaseApp *lastAppBeforeSetting = NULL;
    BaseApp *getAppByName(const uint32_t appid);
    void init(const uint32_t lastAppid);
    void switchApp(BaseApp *app);
    void switchApp(const uint32_t appid)
    {
        switchApp(getAppByName(appid));
    }
    void switchNext()
    {
        int i = 0;
        for (; i < appListLen; i++)
        {
            if (appList[i] == currentApp)
            {
                break;
            }
        }
        i++;
        if(i >= appListLen) i = 0;
        switchApp(appList[i]);
    }
    void loop();
    SemaphoreHandle_t _mutex = NULL;
    SemaphoreHandle_t _binary_switchApp = NULL;
};
extern AppManagerLite appManagerLite;
