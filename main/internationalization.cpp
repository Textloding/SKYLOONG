#include "internationalization.h"
static int current_lang_id = 0; // 0: 中文, 1: 英文
const char *i18n_dict[][2] = {
    {"", ""},
};
static time_t offset = 3600 * 8;
time_t i18n::getNTPOffset()
{
    return offset;
}
void i18n::setNTPOffset(time_t offset_seconds)
{
    offset = offset;
}
void i18n::setLanguage(int lang_id)
{
    current_lang_id = lang_id;
}
int i18n::getLanguage()
{
    return current_lang_id;
}
const char *i18n::getStr(uint32_t id)
{
    return i18n_dict[current_lang_id][id];
}