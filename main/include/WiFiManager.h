#pragma once

class WiFiManager
{
private:
    /* data */
public:
    void init();
    void save();
    uint16_t count();
    void add(const String ssid, const String pass);
    void remove(const String ssid);
    bool has(const String ssid);
    String getPassword(const String ssid);
    void clearall();
    bool autoConnectSaved(uint32_t timeoutMs = 6000, bool scanFirst = true);
    bool requireWiFi(bool forceChoose = false);
    bool requireWiFiForce(bool forceChoose = false)
    {
        for (uint8_t i = 0; i < 3; i++)
        {
            if (requireWiFi(forceChoose) == true)
            {
                return true;
            }
        }
        GUI::toast(_tr(I18N_ID_CANCELLED));
        return false;
    };
    void disconnect()
    {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    };
    bool tryConnectLast();
};
extern WiFiManager WiFiMgr;
