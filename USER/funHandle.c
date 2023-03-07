#include "funHandle.h"
#include "string.h"
#include "ESP8266.h"
#include <stdio.h>
#include "lcd.h"
#include "usartMy.h"
#include "delay.h"
#include "RC522.h"
#include "as608.h"

//开门动作
void openDoorForMQTT(char *data) {
    if (strcmp(data, "open") == 0) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
        uint8_t str[32];
        sprintf((char *) str, "指令开启成功");
        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
        Show_Str_Mid(0, 100, (uint8_t *) str, 16, 240);
        //发送MQTT到服务器
        usart_printf_dma(&ESP_UART,
                         "%c{\"topic\":\"%s\",\"data\":{\"message\":\"open the door\",\"status\":1,\"type\":\"%s\"}}%c",
                         FRAME_HEAD, DEFAULT_PUB_TOPIC, TYPE_COMMAND, FRAME_TAIL);

        // 延时3秒
        delay_ms(3000);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
    } else if (strcmp(data, "close") == 0) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
        uint8_t str[32];
        sprintf((char *) str, "指令关闭成功");
        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
        Show_Str_Mid(0, 100, (uint8_t *) str, 16, 240);
        //发送MQTT到服务器
        usart_printf_dma(&ESP_UART,
                         "%c{\"topic\":\"%s\",\"data\":{\"message\":\"close the door\",\"status\":0,\"type\":\"%s\"}}%c",
                         FRAME_HEAD, DEFAULT_PUB_TOPIC, TYPE_COMMAND, FRAME_TAIL);
    }
}

uint8_t Card_KEY[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};    //{0x11,0x11,0x11,0x11,0x11,0x11};


uint8_t DEFAULT_PASSWORD[16] = {    /*!< 用于每个扇区密码块读写*/
        0x04, 0x05, 0x02, 0x07, 0x02, 0x05,
        0xff, 0x07, 0x80, 0x69,
        0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
};

uint8_t PhoneNFC[][16] = {
        {1, 69, 184, 39},
        {40, 235, 32, 60}
};


void Read_IDcard(void) {
    uint8_t status;  //状态
    uint8_t Card_ID[4];
    uint8_t Card_Type1[2];
    uint8_t Card_Data[20];
    if (MI_OK == PcdRequest(0x52, Card_Type1))  //寻卡函数，如果成功返回 MI_OK  打印多次卡号
    {
        uint16_t cardType = (Card_Type1[0] << 8) | Card_Type1[1];//读不同卡的类型
        usart_printf(&DEBUG_UART, "卡类型：(0x%04X)\r\n", cardType);  //"Card Type(0x%04X):"
        switch (cardType) {
            case 0x4400:
                usart_printf(&DEBUG_UART, "Mifare UltraLight\r\n");
                break;
            case 0x0400:
                usart_printf(&DEBUG_UART, "Mifare One(S50)\r\n");
                break;
            case 0x0200:
                usart_printf(&DEBUG_UART, "Mifare One(S70)\r\n");
                break;
            case 0x0800:
                usart_printf(&DEBUG_UART, "Mifare Pro(X)\r\n");
                break;
            case 0x4403:
                usart_printf(&DEBUG_UART, "Mifare DESFire\r\n");
                break;
            default:
                usart_printf(&DEBUG_UART, "未知卡类型\r\n");
                break;
        }
        status = PcdAnticoll(Card_ID);  //防冲撞 如果成功返回MI_OK
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "ID卡冲撞错误\r\n");
            return;
        }
        usart_printf(&DEBUG_UART, "ID编号为:%02X %02X %02X %02X\r\n", Card_ID[0], Card_ID[1], Card_ID[2],
                     Card_ID[3]);//16进制
        usart_printf(&DEBUG_UART, "ID为:%d %d %d %d \r\n", Card_ID[0], Card_ID[1], Card_ID[2], Card_ID[3]);//十进制
        //sprintf(OK, "ID:%02X%02X%02X%02X", Card_ID[0], Card_ID[1], Card_ID[2], Card_ID[3]);

        status = PcdSelect(Card_ID);  //选卡 如果成功返回MI_OK
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "选卡失败\r\n");
            return;
        }
        // 密码验证
        status = PcdAuthState(PICC_AUTHENT1B, 2, Card_KEY, Card_ID);//验证卡密码 如果成功返回MI_OK
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "三次密码验证失败\r\n");
            usart_printf(&DEBUG_UART, "开始进行ID卡判断\r\n");
            goto CHECKIDS; //返回主页面
        }

        //修改密码
#if 0
        status = changePassword(2, DEFAULT_PASSWORD);
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "修改密码失败\r\n");
            return;
        }
        usart_printf(&DEBUG_UART, "修改密码成功\r\n");
        PcdRead(2, Card_Data);
        usart_printf(&DEBUG_UART, "读取:%s\r\n", Card_Data);
#endif
        //读取扇区 2 密码数据
#if 1
        status = PcdRead(1, Card_Data);
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "读取扇区2密码数据失败\r\n");
            return;
        }
        // 比较前6个字节是否相同
        if (memcmp(Card_Data, DEFAULT_PASSWORD, 6) != 0) {
            usart_printf(&DEBUG_UART, "密码验证失败\r\n");
            return;
        }
        usart_printf(&DEBUG_UART, "密码验证成功\r\n");
#endif

        //卡片进入休眠状态
        status = PcdHalt();
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "ID卡休眠失败\r\n");
            return;
        }
        usart_printf(&DEBUG_UART, "ID卡休眠\r\n");

        CHECKIDS:
        if (!authStateNFC(Card_ID)) {
            return;
        }
        usart_printf(&DEBUG_UART, "通过验证ID号成功\r\n");
    }

}


/**
 *定于验证NFC卡的密码,返回验证结果
* @param pKey 数组存储的密码
 * @param pSnr
 * @return
 */
uint8_t authStateNFC(const uint8_t pKey[]) {
    //动态获取二维数组长度
    uint16_t len = sizeof(PhoneNFC) / sizeof(PhoneNFC[0]);
    for (int i = 0; i < len; i++) {
        uint8_t c = 0;
        for (int j = 0; j < 4; ++j) {
            //幻化为10进制
            if (PhoneNFC[i][j] == pKey[j]) {
                if (++c >= 4) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
