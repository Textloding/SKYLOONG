#include "A_Config.h"
#include "WiFiManager.h"

WiFiManager WiFiMgr;

static String WiFiTable[WIFI_SAVE_MAX][2];
static uint16_t WiFiCount = 0;
extern lv_indev_t *indev_keypad; // 用于临时切换group
static int16_t findSavedIndex(const String &ssid)
{
    for (uint16_t i = 0; i < WiFiCount; i++)
    {
        if (WiFiTable[i][0] == ssid)
        {
            return i;
        }
    }
    return -1;
}

static bool connectSavedIndex(uint16_t index, uint32_t timeoutMs)
{
    if (index >= WiFiCount || WiFiTable[index][0] == "")
        return false;

    String ssid = WiFiTable[index][0];
    String pass = WiFiTable[index][1];
    ESP_LOGI("WiFiMgr", "auto connect to saved WiFi: %s", ssid.c_str());

    WiFi.disconnect(false);
    WiFi.mode(WIFI_STA);
    delay(50);

    if (pass.length() == 0)
    {
        WiFi.begin(ssid.c_str());
    }
    else
    {
        WiFi.begin(ssid.c_str(), pass.c_str());
    }

    if (WiFi.waitForConnectResult(timeoutMs) == WL_CONNECTED)
    {
        ESP_LOGI("WiFiMgr", "auto connect success: %s, ip=%s", ssid.c_str(), WiFi.localIP().toString().c_str());
        return true;
    }

    ESP_LOGW("WiFiMgr", "auto connect failed: %s", ssid.c_str());
    WiFi.disconnect(false);
    return false;
}

void WiFiManager::save()
{
    File f = LittleFS.open("/.wifi.csv", "w");
    if (f)
    {
        for (uint16_t i = 0; i < WiFiCount; i++)
        {
            if (WiFiTable[i][0] != "")
            {
                String s = WiFiTable[i][0] + "," + WiFiTable[i][1] + "\n";
                f.print(s);
            }
        }
        f.close();
    }
}

uint16_t WiFiManager::count() {
    return WiFiCount;
}

void WiFiManager::add(const String ssid, const String pass)
{
    if (ssid == "")
        return;

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
        while (WiFiCount < WIFI_SAVE_MAX)
        {
            String line = f.readStringUntil('\n');
            line.replace("\r", "");
            line.replace("\n", "");
            if (line != "")
            {
                int comma = line.indexOf(',');
                if (comma > 0)
                {
                    WiFiTable[WiFiCount][0] = line.substring(0, comma);
                    WiFiTable[WiFiCount][1] = line.substring(comma + 1);
                    ++WiFiCount;
                }
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

bool WiFiManager::autoConnectSaved(uint32_t timeoutMs, bool scanFirst)
{
    if (WiFi.isConnected())
        return true;

    if (WiFiCount == 0)
    {
        ESP_LOGI("WiFiMgr", "auto connect skipped: no saved WiFi");
        return false;
    }

    wifi_mode_t mode = WiFi.getMode();
    if (mode == WIFI_AP && hal.server_started)
    {
        ESP_LOGW("WiFiMgr", "auto connect skipped: web server is running in AP mode");
        return false;
    }

    if (scanFirst)
    {
        int16_t candidateIndex[WIFI_SAVE_MAX];
        int32_t candidateRssi[WIFI_SAVE_MAX];
        uint16_t candidateCount = 0;

        for (uint16_t i = 0; i < WIFI_SAVE_MAX; i++)
        {
            candidateIndex[i] = -1;
            candidateRssi[i] = -1000;
        }

        WiFi.disconnect(false);
        WiFi.mode(WIFI_STA);
        int16_t n = WiFi.scanNetworks(false, false);

        if (n > 0)
        {
            for (int16_t scanIndex = 0; scanIndex < n; scanIndex++)
            {
                String ssid = WiFi.SSID(scanIndex);
                int16_t savedIndex = findSavedIndex(ssid);
                if (savedIndex < 0)
                    continue;

                bool found = false;
                for (uint16_t i = 0; i < candidateCount; i++)
                {
                    if (candidateIndex[i] == savedIndex)
                    {
                        if (WiFi.RSSI(scanIndex) > candidateRssi[i])
                            candidateRssi[i] = WiFi.RSSI(scanIndex);
                        found = true;
                        break;
                    }
                }

                if (!found && candidateCount < WIFI_SAVE_MAX)
                {
                    candidateIndex[candidateCount] = savedIndex;
                    candidateRssi[candidateCount] = WiFi.RSSI(scanIndex);
                    candidateCount++;
                }
            }

            for (uint16_t i = 0; i < candidateCount; i++)
            {
                for (uint16_t j = i + 1; j < candidateCount; j++)
                {
                    if (candidateRssi[j] > candidateRssi[i])
                    {
                        int16_t tmpIndex = candidateIndex[i];
                        int32_t tmpRssi = candidateRssi[i];
                        candidateIndex[i] = candidateIndex[j];
                        candidateRssi[i] = candidateRssi[j];
                        candidateIndex[j] = tmpIndex;
                        candidateRssi[j] = tmpRssi;
                    }
                }
            }
        }

        WiFi.scanDelete();

        if (candidateCount == 0 && n >= 0)
        {
            ESP_LOGI("WiFiMgr", "auto connect skipped: no saved WiFi found in scan");
            return false;
        }

        for (uint16_t i = 0; i < candidateCount; i++)
        {
            if (connectSavedIndex(candidateIndex[i], timeoutMs))
                return true;
        }

        if (n >= 0)
            return false;

        ESP_LOGW("WiFiMgr", "auto connect scan failed, trying saved WiFi in stored order");
        WiFi.scanDelete();
        for (uint16_t i = 0; i < WiFiCount; i++)
        {
            if (connectSavedIndex(i, timeoutMs))
                return true;
        }
        return false;
    }

    for (uint16_t i = 0; i < WiFiCount; i++)
    {
        if (connectSavedIndex(i, timeoutMs))
            return true;
    }

    return false;
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
        if (autoConnectSaved(5000))
        {
            GUI::toast(_tr(I18N_ID_CONNECTED));
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
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    else
    {
        return true;
    }
    return false;
}

bool WiFiManager::tryConnectLast()
{
    if (WiFi.isConnected())
        return true;
    return autoConnectSaved(5000);
}
