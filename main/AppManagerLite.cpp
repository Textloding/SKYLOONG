#include <A_Config.h>
#include "AppManagerLite.h"

BaseApp *appList[50];
int appListLen = 0;
void add_to_app_list(BaseApp *app)
{
    appList[appListLen++] = app;
    ESP_LOGI("AppManagerLite", "Add %u to app list", app->appid);
}

AppManagerLite appManagerLite;

BaseApp *AppManagerLite::getAppByName(const uint32_t appid)
{
    if (appid == 50)
        return appSettings;
    for (int i = 0; i < appListLen; i++)
    {
        if (appList[i]->appid == appid)
        {
            return appList[i];
        }
    }
    ESP_LOGW("AppManagerLite", "App %u not found", appid);
    return NULL;
}
void task_app_loop(void *p)
{
    while (1)
    {
        appManagerLite.loop();
        vTaskDelay(10);
    }
}
static uint32_t last_switch_millis = 0;
static uint8_t last_overflow_appid = 1;     // 上次保存的appid
void task_app_switch(void *p)
{
    while (1)
    {
        xSemaphoreTake(appManagerLite._binary_switchApp, portMAX_DELAY);
        ESP_LOGI("AppManagerLite", "got signal");
        appManagerLite.switchNext();
    }
}
void AppManagerLite::init(const uint32_t lastAppid)
{
    _mutex = xSemaphoreCreateMutex();
    _binary_switchApp = xSemaphoreCreateBinary();
    xSemaphoreTake(_binary_switchApp, 0);
    BaseApp *lastApp = getAppByName(lastAppid);
    ESP_LOGI("AppManagerLite", "App Count: %d", appListLen);
    if (lastApp != NULL)
    {
        switchApp(lastApp);
        last_overflow_appid = lastAppid;
    }
    else
    {
        switchApp(1);
        last_overflow_appid = lastAppid;
    }
    xTaskCreatePinnedToCore(task_app_loop, "task_app_loop", 8192, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(task_app_switch, "task_app_switch", 4096, NULL, 4, NULL, 0);
    xTaskCreate([](void *p)
                {
                    while(1)
                    {
                        if (millis() - last_switch_millis > 30000)
                        {
                            if ((last_overflow_appid != last_appid) && (last_appid != 50))
                            {
                                last_overflow_appid = last_appid;
                                hal.pref.putInt("last_appid", last_appid);
                            }
                        } vTaskDelay(1000);} },
                "task_app_save", 4096, NULL, 1, NULL);
}
void AppManagerLite::switchApp(BaseApp *app)
{
    if (app == NULL)
    {
        return;
    }
    last_appid = app->appid;
    if (currentApp)
        currentApp->stop();
    ESP_LOGI("AppManagerLite", "stop_req");
    xSemaphoreTake(_mutex, portMAX_DELAY);
    if (millis() - last_switch_millis < 400)
    {
        vTaskDelay(400 - (millis() - last_switch_millis));
    }
    last_switch_millis = millis();
    ESP_LOGI("AppManagerLite", "destroy begin");
    if (currentApp != NULL)
    {
        currentApp->destroy();
        currentApp->_appScreen = NULL;
    }
    currentApp = app;
    ESP_LOGI("AppManagerLite", "switch begin");
    hal.LOCKLV();
    currentApp->_appScreen = lv_obj_create(NULL);
    lv_scr_load_anim(currentApp->_appScreen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
    hal.UNLOCKLV();
    currentApp->setup();
    ESP_LOGI("AppManagerLite", "Switch to %u", currentApp->appid);
    xSemaphoreGive(_mutex);
}

void AppManagerLite::loop()
{
    if (currentApp != NULL)
    {
        xSemaphoreTake(_mutex, portMAX_DELAY);
        currentApp->loop();
        xSemaphoreGive(_mutex);
    }
}
