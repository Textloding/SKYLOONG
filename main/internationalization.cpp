#include "internationalization.h"
static int current_lang_id = 0; // 0: 中文, 1: 英文
const char *i18n_dict[][2] = {
    {"12小时制", "12-hour"},
    {"以12小时制显示时间", "Display time in 12-hour format"},
    {"显示电量", "Battery level"},
    {"显示具体电量而不是电池图标", "Display specific battery level instead of battery icon"},
    {"状态栏居中", "Center status bar"},
    {"使状态栏图标居中显示", "Center align status bar icons"},
    {"开机动画", "Boot animation"},
    {"开机时显示动画", "Display animation on boot"},
    {"网页服务器", "Web server"},
    {"启动网页服务器，如需离线设置时间请点这里", "Start web server. Also click here for offline time setting"},
    {"启动", "Start"},
    {"停止", "Stop"},
    {"同步时间", "Sync time"},
    {"从NTP服务器同步时间", "Sync time from NTP server"},
    {"同步", "Sync"},
    {"亮度调节", "Brightness"},
    {"时区", "Time zone"},
    {"设置时区", "Set time zone"},
    {"恢复出厂设置", "Factory reset"},
    {"将设备恢复至出厂状态", "Restore the device to factory state"},
    {"恢复出厂", "Reset"},
    {"正在重置", "Resetting"},
    {"再按%d次后重置", "After %d"},
    {"同步成功", "Sync successful"},
    {"同步失败", "Sync failed"},
    {"无法连接TCP", "Unable to connect to TCP"},
    {"进入离线模式", "Enter offline mode"},
    {"TCP连接断开", "TCP connection disconnected"},
    {"打开文件夹", "Open folder"},
    {"请选择文件", "Select a file to play."},
    {"文件打开失败", "Failed to open file"},
    {"已取消", "Cancelled"},
    {"成功", "Success"},
    {"连接失败", "Connection failed"},
    {"用户已取消", "User cancelled"},
    {"选择WiFi", "Select WiFi"},
    {"正在扫描...", "Scanning..."},
    {"WiFi扫描失败", "WiFi scan failed"},
    {"无网络", "No network"},
    {"密码不符合要求", "Password too short"},
    {"连接到：%s", "Connected to: %s"},
    {"请输入密码", "Please enter password"},
    {"正在连接到：%s", "Connecting to: %s"},
    {"需要进行操作", "Attention"},
    {"  此功能要求进入“设置模式”，但键盘未提供相关API，请通过组合键手动进入此模式。", "This feature requires entering \"Settings Mode\". Please enter this mode manually with combination keys."},
    {"服务器已启动", "Server started"},
    {"服务器已关闭", "Server closed"},
    {"WiFi列表已满", "WiFi list is full"},
    {"已连接", "Connected"},
    {"正在连接WiFi", "Connecting to WiFi"},
    {"重新选择WiFi", "Reselect WiFi"},
    {"重新扫描附近的WiFi并连接", "Rescan nearby WiFi and connect"},
    {"扫描", "Scan"},
    {"主题", "Themes"},
    {"修改主题", "Change themes"},
    {"默认\n斯巴达勇士\n九尾狐", "Default\nSpartan\nFox"},
};
static time_t offset = 3600 * 8;
time_t i18n::getNTPOffset()
{
    return offset;
}
void i18n::setNTPOffset(time_t offset_seconds)
{
    offset = offset_seconds;
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
    return i18n_dict[id][current_lang_id];
}