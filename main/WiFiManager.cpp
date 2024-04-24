#include "A_Config.h"
#include "WiFiManager.h"

WiFiManager WiFiMgr;

static String WiFiTable[WIFI_SAVE_MAX][2];
static uint16_t WiFiCount = 0;
extern lv_indev_t *indev_keypad; // 用于临时切换group
void WiFiManager::save()
{
    File f = LittleFS.open("/.wifi.csv", "w");
    if (f)
    {
        for (uint16_t i = 0; i < WiFiCount; i++)
        {
            if (WiFiTable[i][0] != "" && WiFiTable[i][1].length() >= 8)
            {
                String s = WiFiTable[i][0] + "," + WiFiTable[i][1] + "\n";
                f.print(s);
            }
        }
        f.close();
    }
}

void WiFiManager::add(const String ssid, const String pass)
{
    for (uint16_t i = 0; i < WiFiCount; i++)
    {
        if (WiFiTable[i][0] == ssid)
        {
            WiFiTable[i][1] = pass;
            save();
            return;
        }
    }
    if (WiFiCount < WIFI_SAVE_MAX)
    {
        WiFiTable[WiFiCount][0] = ssid;
        WiFiTable[WiFiCount][1] = pass;
        ++WiFiCount;
        save();
    }
    return;
}

void WiFiManager::remove(const String ssid)
{
    for (uint16_t i = 0; i < WiFiCount; i++)
    {
        if (WiFiTable[i][0] == ssid)
        {
            for (uint16_t j = i; j < WiFiCount - 1; j++)
            {
                WiFiTable[j][0] = WiFiTable[j + 1][0];
                WiFiTable[j][1] = WiFiTable[j + 1][1];
            }
            --WiFiCount;
            save();
            return;
        }
    }
    return;
}

bool WiFiManager::has(const String ssid)
{
    for (uint16_t i = 0; i < WiFiCount; i++)
    {
        if (WiFiTable[i][0] == ssid)
        {
            return true;
        }
    }
    return false;
}

String WiFiManager::getPassword(const String ssid)
{
    for (uint16_t i = 0; i < WiFiCount; i++)
    {
        if (WiFiTable[i][0] == ssid)
        {
            return WiFiTable[i][1];
        }
    }
    return "";
}

void WiFiManager::clearall()
{
    LittleFS.remove("/.wifi.csv");
    WiFiCount = 0;
    for (uint16_t i = 0; i < WIFI_SAVE_MAX; i++)
    {
        WiFiTable[i][0] = "";
        WiFiTable[i][1] = "";
    }
}

void WiFiManager::init()
{
    WiFiCount = 0;
    for (uint16_t i = 0; i < WIFI_SAVE_MAX; i++)
    {
        WiFiTable[i][0] = "";
        WiFiTable[i][1] = "";
    }
    File f = LittleFS.open("/.wifi.csv", "r");
    if (f)
    {
        while (1)
        {
            String line = f.readStringUntil('\n');
            if (line != "")
            {
                WiFiTable[WiFiCount][0] = line.substring(0, line.indexOf(','));
                WiFiTable[WiFiCount][1] = line.substring(line.indexOf(',') + 1);
                WiFiTable[WiFiCount][1].replace("\n", "");
                ++WiFiCount;
            }
            else
                break;
        }
        f.close();
    }
    else
    {
        WiFiCount = 0;
        File f = LittleFS.open("/.wifi.csv", "w");
        if (f)
        {
            f.close();
        }
        else
        {
            ESP_LOGE("WiFiMgr", "无法写入文件.wifi.csv");
        }
    }
}

bool WiFiManager::requireWiFi(bool forceChoose)
{
    hal.LOCKLV();
    lv_group_t *last_group = lv_group_get_default();
    lv_group_t *group;
    hal.UNLOCKLV();
    if (WiFi.status() != WL_CONNECTED)
    {
        if (forceChoose)
            goto choose;
        GUI::toast(_tr(I18N_ID_CONNECTING));
        WiFi.begin();
        if (WiFi.waitForConnectResult(5000) == WL_CONNECTED)
        {
            GUI::toast(_tr(I18N_ID_CONNECTED));
            hal.pref.putBool("wifi_succ", true);
            return true;
        }
        else
        {
        choose:
            char ssid[64];
            char passwd[64];
            hal.LOCKLV();
            group = lv_group_create();
            lv_group_set_wrap(group, false);
            lv_group_set_default(group);
            lv_indev_set_group(indev_keypad, group);
            lv_obj_t *scr_store = lv_scr_act();
            hal.UNLOCKLV();
            int res = esp_dpp_start(ssid, passwd);
            hal.LOCKLV();
            lv_scr_load_anim(scr_store, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
            lv_indev_set_group(indev_keypad, last_group);
            lv_group_set_default(last_group);
            lv_group_del(group);
            hal.UNLOCKLV();
            vTaskDelay(320);
            if (res == 0)
            {
                if (WiFiCount == WIFI_SAVE_MAX)
                    GUI::toast(_tr(I18N_ID_WIFI_LIST_FULL));
                add(ssid, passwd);
                hal.pref.putBool("wifi_succ", true);
                return true;
            }
            else
            {
                hal.pref.putBool("wifi_succ", false);
                return false;
            }
        }
    }
    else
    {
        hal.pref.putBool("wifi_succ", true);
        return true;
    }
    hal.pref.putBool("wifi_succ", false);
    return false;
}

bool WiFiManager::tryConnectLast()
{
    if (WiFi.isConnected())
        return true;
    bool wifi_succ = hal.pref.getBool("wifi_succ", false);
    if (wifi_succ == false)
        return false;
    WiFi.begin();
    if (WiFi.waitForConnectResult(5000) != WL_CONNECTED)
    {
        hal.pref.putBool("wifi_succ", false);
        return false;
    }
    return true;
}
