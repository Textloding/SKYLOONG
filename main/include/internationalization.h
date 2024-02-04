#pragma once
#include "Arduino.h"
#include "i18n_defines.h"
extern "C" const char *i18n_dict[][2];
namespace i18n
{
    time_t getNTPOffset();
    void setNTPOffset(time_t offset_seconds);
    void setLanguage(int lang_id);
    int getLanguage();
    const char *getStr(uint32_t id);
};

