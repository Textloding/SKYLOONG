#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

struct WeatherCacheData
{
    char location[32];
    uint8_t weatherCode_today;
    uint8_t weatherCode_tomorrow;
    uint8_t weatherCode_tomorrow_1;
    int8_t temperature_now;
    char index_wear[16];
    char index_sport[16];
    char index_flu[16];
    char index_car[16];
};

static_assert(sizeof(WeatherCacheData) == 100, "Weather cache format changed");

inline bool weatherCacheFieldIsTerminated(const char *field, size_t size)
{
    return field != nullptr && memchr(field, '\0', size) != nullptr;
}

inline bool weatherCacheIsValid(const WeatherCacheData &data)
{
    return data.weatherCode_today < 40 &&
           data.weatherCode_tomorrow < 40 &&
           data.weatherCode_tomorrow_1 < 40 &&
           weatherCacheFieldIsTerminated(data.location, sizeof(data.location)) &&
           weatherCacheFieldIsTerminated(data.index_wear, sizeof(data.index_wear)) &&
           weatherCacheFieldIsTerminated(data.index_sport, sizeof(data.index_sport)) &&
           weatherCacheFieldIsTerminated(data.index_flu, sizeof(data.index_flu)) &&
           weatherCacheFieldIsTerminated(data.index_car, sizeof(data.index_car));
}
