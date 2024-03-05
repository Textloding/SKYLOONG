#include "A_Config.h"
extern lv_obj_t *ta_debug;
typedef struct protocol_t
{
    uint16_t len;
    uint8_t type;
    uint8_t *data;
    uint8_t crc;
} protocol_t;

#define PROTOCOL_STX 0x02
#define PROTOCOL_ETX 0x03
#define PROTOCOL_ACK 0x06
#define PROTOCOL_TYPE_VERSION 0x1A
#define PROTOCOL_TYPE_DISP_MODE 0x1C
#define PROTOCOL_TYPE_STATUS 0x1D
#define PROTOCOL_TYPE_ECHO 0x16
#define PROTOCOL_TYPE_MENU 0xA3
#define PROTOCOL_TYPE_PARAM 0xA0
#define PROTOCOL_TYPE_TIME 0x1E
#define PROTOCOL_TYPE_BATTERY 0x1F
static bool screen_is_on = true;
static bool in_setting_mode = false;
static uint8_t getcrc(protocol_t *buff)
{
    uint8_t crc = 0;
    crc ^= (buff->len) >> 8;
    crc ^= (buff->len) & 0xFF;
    crc ^= buff->type;
    if (buff->data)
    {
        for (uint16_t i = 0; i < buff->len - 1; i++)
        {
            crc ^= buff->data[i];
        }
    }
    crc ^= PROTOCOL_ETX;
    return crc;
}
static uint8_t getByte()
{
    while (Serial2.available() == 0)
        delay(10);
    char c = Serial2.read();
    return c;
}
void sendPkt(protocol_t *pkt)
{
    uint8_t crc = getcrc(pkt);
    Serial2.write(PROTOCOL_STX);
    Serial2.write(pkt->len >> 8);
    Serial2.write(pkt->len & 0xFF);
    Serial2.write(pkt->type);
    if (pkt->data)
    {
        Serial2.write(pkt->data, pkt->len - 1);
    }
    Serial2.write(PROTOCOL_ETX);
    Serial2.write(crc);
    Serial2.flush();
}

void parasePkt(protocol_t *pkt)
{
    static protocol_t pkt_send;
    if (pkt_send.data == NULL)
        pkt_send.data = (uint8_t *)malloc(128);
    pkt_send.type = pkt->type;
    memset(pkt_send.data, 0, 128);
    switch (pkt->type)
    {
    case PROTOCOL_TYPE_VERSION:
        pkt_send.len = 1 + 2 + strlen(FIRMWARE_VERSION) + 1;
        strcpy((char *)(pkt_send.data + 2), FIRMWARE_VERSION);
        sendPkt(&pkt_send);
        break;
    case PROTOCOL_TYPE_DISP_MODE:
        pkt_send.len = 1 + 2;
        sendPkt(&pkt_send);
        break;
    case PROTOCOL_TYPE_STATUS:
    {
        kb_status_t *kb_status = (kb_status_t *)pkt->data;
        memcpy(&hal.kb_status, kb_status, sizeof(kb_status_t));
        hal.send_sysctl(EVENT_KB_STATUS_CHANGED, 0);
        pkt_send.len = 1 + 2;
        sendPkt(&pkt_send);
        break;
    }
    case PROTOCOL_TYPE_ECHO:
        if (pkt->len == 2)
        {
            hal.send_sysctl(EVENT_APM_CHANGED, pkt->data[0]);
        }
        memcpy(pkt_send.data + 2, pkt->data, pkt->len - 1);
        pkt_send.len = 2 + pkt->len;
        sendPkt(&pkt_send);
        break;
    case PROTOCOL_TYPE_MENU: // 开关屏幕在这里
    {
        uint8_t data = pkt->data[0];
        uint8_t send_data = 255;
        if (in_setting_mode == false && data != 0xA0 && data != 0xAF && data != 0xAE)
        {
            in_setting_mode = !in_setting_mode;
            hal.send_sysctl(EVENT_GOTO_SETTING);
        }
        switch (data)
        {
        case 0xA0: // 进入编辑模式（右下角两个同时长按）
            in_setting_mode = !in_setting_mode;
            if (in_setting_mode)
                hal.send_sysctl(EVENT_GOTO_SETTING);
            else
                hal.send_sysctl(EVENT_EXIT_SETTING);
            break;
        case 0xA1: // BackSpace
            //xSemaphoreGive(appManagerLite._binary_switchApp);
            break;
        case 0xA2: // 方向键向左
            send_data = LV_KEY_LEFT;
            break;
        case 0xA3: // 方向键向右
            send_data = LV_KEY_RIGHT;
            break;
        case 0xA4: // 方向键向上
            if (hal.lv_has_kb == true)
                send_data = LV_KEY_UP;
            else
                send_data = LV_KEY_PREV;
            break;
        case 0xA5: // 方向键向下
            if (hal.lv_has_kb == true)
                send_data = LV_KEY_DOWN;
            else
                send_data = LV_KEY_NEXT;
            break;
        case 0xAA: // Enter
            send_data = LV_KEY_ENTER;
            break;
        case 0xAE: // 菜单间切换（右下角+左上角短按）
            xSemaphoreGive(appManagerLite._binary_switchApp);
            hal.setBrightness(hal._brightness);
            screen_is_on = true;
            break;
        case 0xAF: // 开关屏幕（这个无论是开还是关都是一种数据，右下+左上短按）
            screen_is_on = !screen_is_on;
            hal.send_sysctl(EVENT_TOGGLE_SCREEN_ON, screen_is_on);
            break;
        default:
            break;
        }
        if (send_data != 255)
        {
            xQueueSend(hal._queue_kb, &send_data, 0);
        }
    }
        pkt_send.len = 2 + pkt->len;
        memcpy(pkt_send.data + 2, pkt->data, pkt->len - 1);
        sendPkt(&pkt_send);
        break;
    case PROTOCOL_TYPE_PARAM:
        hal.send_sysctl(EVENT_GOSLEEP);
        pkt_send.len = 2 + pkt->len;
        memcpy(pkt_send.data + 2, pkt->data, pkt->len - 1);
        sendPkt(&pkt_send);
        break;
    case PROTOCOL_TYPE_TIME:
        pkt_send.len = 2 + pkt->len;
        memcpy(pkt_send.data + 2, pkt->data, pkt->len - 1);
        sendPkt(&pkt_send);
        break;
    case PROTOCOL_TYPE_BATTERY:
    {
        kb_battery_t *kb_battery = (kb_battery_t *)pkt->data;
        hal.battery_pct = kb_battery->value * 2;
        hal.battery_status = kb_battery->status;
        pkt_send.len = 2 + pkt->len;
        memcpy(pkt_send.data + 2, pkt->data, pkt->len - 1);
        sendPkt(&pkt_send);
        break;
    }
    default:
        break;
    }
}
void getPkt()
{
    static protocol_t pkt;
    memset(&pkt, 0, sizeof(protocol_t));
    while (getByte() != PROTOCOL_STX)
        delay(10);
    // 此时已经收到了STX
    uint16_t tmp = getByte();
    tmp = tmp << 8;
    tmp |= getByte();
    pkt.len = tmp;
    pkt.type = getByte();
    if (pkt.len > 0)
    {
        pkt.data = (uint8_t *)malloc(pkt.len);
        Serial2.readBytes(pkt.data, pkt.len - 1);
    }
    if (getByte() != PROTOCOL_ETX)
    {
        free(pkt.data);
        return;
    }
    pkt.crc = getByte();
    if (pkt.crc != getcrc(&pkt))
    {
        free(pkt.data);
        return;
    }
    Serial2.write(PROTOCOL_ACK);
    Serial2.flush();
    parasePkt(&pkt);
    return;
}
void task_protocol(void *pvParameters)
{
    while (1)
    {
        if (Serial2.available() == 0)
        {
            delay(50);
            continue;
        }
        getPkt();
        delay(1);
    }
}

void protocol_init()
{
    pinMode(PIN_SERIAL2_RX, INPUT_PULLUP);
    pinMode(PIN_SERIAL2_TX, OUTPUT);
    Serial2.begin(115200, SERIAL_8N1, PIN_SERIAL2_RX, PIN_SERIAL2_TX);
    xTaskCreatePinnedToCore(task_protocol, "task_protocol", 4096, NULL, 25, NULL, 1);
}
