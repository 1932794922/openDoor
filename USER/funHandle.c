#include "funHandle.h"
#include "string.h"
#include "ESP8266.h"
#include <stdio.h>
#include "lcd.h"
#include "usartMy.h"
#include "delay.h"
#include "RC522.h"
#include "as608.h"

//���Ŷ���
void openDoorForMQTT(char *data) {
    if (strcmp(data, "open") == 0) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
        uint8_t str[32];
        sprintf((char *) str, "ָ����ɹ�");
        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
        Show_Str_Mid(0, 100, (uint8_t *) str, 16, 240);
        //����MQTT��������
        usart_printf_dma(&ESP_UART,
                         "%c{\"topic\":\"%s\",\"data\":{\"message\":\"open the door\",\"status\":1,\"type\":\"%s\"}}%c",
                         FRAME_HEAD, DEFAULT_PUB_TOPIC, TYPE_COMMAND, FRAME_TAIL);

        // ��ʱ3��
        delay_ms(3000);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
    } else if (strcmp(data, "close") == 0) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
        uint8_t str[32];
        sprintf((char *) str, "ָ��رճɹ�");
        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
        Show_Str_Mid(0, 100, (uint8_t *) str, 16, 240);
        //����MQTT��������
        usart_printf_dma(&ESP_UART,
                         "%c{\"topic\":\"%s\",\"data\":{\"message\":\"close the door\",\"status\":0,\"type\":\"%s\"}}%c",
                         FRAME_HEAD, DEFAULT_PUB_TOPIC, TYPE_COMMAND, FRAME_TAIL);
    }

    LCD_Fill(0, 100, lcddev.width, 160, BLACK);

}

/**
 * RCC���Ž���
 * @param nms
 */
void openDoorRC522(uint16_t nms, uint8_t *id, uint8_t *openType) {

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
    uint8_t str[32];
    sprintf((char *) str, "ˢ�������ɹ�");
    LCD_Fill(0, 100, lcddev.width, 160, BLACK);
    Show_Str_Mid(0, 100, (uint8_t *) str, 16, 240);
    //����MQTT��������
    usart_printf_dma(&ESP_UART,
                     "%c{\"topic\":\"%s\",\"data\":{\"message\":\"%s\",\"status\":1,\"type\":\"%s\",\"id\":\"%s\"}}%c",
                     FRAME_HEAD, DEFAULT_PUB_TOPIC, openType, TYPE_RFID, id, FRAME_TAIL);
//    usart_printf(&DEBUG_UART,
//                 "%c{\"topic\":\"%s\",\"data\":{\"message\":\"%s\",\"status\":1,\"type\":\"%s\",\"id\":\"%s\"}}%c",
//                 FRAME_HEAD, DEFAULT_PUB_TOPIC, openType, TYPE_RFID, id, FRAME_TAIL);
    // ��ʱ3��
    delay_ms(nms);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
    LCD_Fill(0, 100, lcddev.width, 160, BLACK);
}

uint8_t DEFAULT_PASSWORD[16] = {    /*!< ����ÿ������������д*/
        0x04, 0x05, 0x02, 0x07, 0x02, 0x05,
        0xff, 0x07, 0x80, 0x69,
        0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
};


uint8_t DELETE_PASSWORD[16] = {    /*!< ����ÿ������������д*/
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x07, 0x80, 0x69,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

//id����¼
uint8_t PhoneNFC[][16] = {
        {1,  69,  184, 39},
        {40, 235, 32,  61}
};

uint8_t cardTypeStr[32];
uint8_t Card_Type[2];

void Read_IDcard(void) {

    if (MI_OK == PcdRequest(0x52, Card_Type))  //Ѱ������������ɹ����� MI_OK  ��ӡ��ο���
    {
        uint8_t Card_KEY[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};    //{0x11,0x11,0x11,0x11,0x11,0x11};
        uint8_t status;  //״̬
        uint8_t Card_ID[4];
        uint8_t Card_Data[20];
        uint16_t cardType = (Card_Type[0] << 8) | Card_Type[1];//����ͬ��������
        switch (cardType) {
            case 0x4400:
                // usart_printf(&DEBUG_UART, "Mifare UltraLight\r\n");
                sprintf((char *) cardTypeStr, "Mifare UltraLight");
                break;
            case 0x0400:
                //usart_printf(&DEBUG_UART, "Mifare One(S50)\r\n");
                sprintf((char *) cardTypeStr, "One(S50)");
                break;
            case 0x0200:
                //usart_printf(&DEBUG_UART, "Mifare One(S70)\r\n");
                sprintf((char *) cardTypeStr, "One(S70)");
                break;
            case 0x0800:
                // usart_printf(&DEBUG_UART, "Mifare Pro(X)\r\n");
                sprintf((char *) cardTypeStr, "Pro(X)");

                break;
            case 0x4403:
                // usart_printf(&DEBUG_UART, "Mifare DESFire\r\n");
                sprintf((char *) cardTypeStr, "DESFire");

                break;
            default:
                // usart_printf(&DEBUG_UART, "δ֪������\r\n");
                sprintf((char *) cardTypeStr, "unknown");
                break;
        }
        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
        status = PcdAnticoll(Card_ID);  //����ײ ����ɹ�����MI_OK
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "ID����ײ����\r\n");
            LCD_Fill(0, 100, lcddev.width, 160, BLACK);
            Show_Str_Mid(0, 100, (uint8_t *) "ID����ײ", 16, 240);
            return;
        }
        usart_printf(&DEBUG_UART, "ID���Ϊ:%02X %02X %02X %02X\r\n", Card_ID[0], Card_ID[1], Card_ID[2],
                     Card_ID[3]);//16����
        uint8_t ids[9];
        sprintf((char *) ids, "%02X%02X%02X%02X", Card_ID[0], Card_ID[1], Card_ID[2], Card_ID[3]);
        usart_printf(&DEBUG_UART, "IDΪ:%d %d %d %d \r\n", Card_ID[0], Card_ID[1], Card_ID[2], Card_ID[3]);//ʮ����
        status = PcdSelect(Card_ID);  //ѡ�� ����ɹ�����MI_OK
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "ѡ��ʧ��\r\n");
            return;
        }
        // ������֤
        status = PcdAuthState(PICC_AUTHENT1B, 2, Card_KEY, Card_ID);//��֤������ ����ɹ�����MI_OK
        if (status != MI_OK) {
            // usart_printf(&DEBUG_UART, "����������֤ʧ��\r\n");
            // usart_printf(&DEBUG_UART, "��ʼ����ID���ж�\r\n");
            if (!authStateNFC(Card_ID)) {
                usart_printf(&DEBUG_UART, "��Ч��\r\n");
                LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                Show_Str_Mid(0, 100, (uint8_t *) "��Ч��", 16, 240);
                errorRFIDMQTT(ids, (uint8_t *) "invalid card");
                return;
            }
            //  usart_printf(&DEBUG_UART, "ͨ����֤ID�ųɹ�\r\n");
            openDoorRC522(3000, ids, (uint8_t *) cardTypeStr);
            return;
        }

        //�޸�����
#if 0
        status = changePassword(2, DEFAULT_PASSWORD);
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "�޸�����ʧ��\r\n");
            return;
        }
        usart_printf(&DEBUG_UART, "�޸�����ɹ�\r\n");
        PcdRead(2, Card_Data);
        usart_printf(&DEBUG_UART, "��ȡ:%s\r\n", Card_Data);
#endif
        //��ȡ���� 2 ��������
#if 1
        status = PcdRead(1, Card_Data);
        if (status != MI_OK) {
            // usart_printf(&DEBUG_UART, "��ȡ����2��������ʧ��\r\n");
            usart_printf(&DEBUG_UART, "��Ч��\r\n");
            LCD_Fill(0, 100, lcddev.width, 160, BLACK);
            Show_Str_Mid(0, 100, (uint8_t *) "��Ч��", 16, 240);
            errorRFIDMQTT(ids, (uint8_t *) "invalid card");
            return;
        }
        // �Ƚ�ǰ6���ֽ��Ƿ���ͬ
        if (memcmp(Card_Data, DEFAULT_PASSWORD, 6) != 0) {
            // usart_printf(&DEBUG_UART, "������֤ʧ��\r\n");
            LCD_Fill(0, 100, lcddev.width, 160, BLACK);
            Show_Str_Mid(0, 100, (uint8_t *) "������֤ʧ��", 16, 240);
            errorRFIDMQTT(ids, (uint8_t *) "password verification failed");
            return;
        }
        // usart_printf(&DEBUG_UART, "������֤�ɹ�\r\n");

#endif

        //��Ƭ��������״̬
        status = PcdHalt();
        if (status != MI_OK) {
            //usart_printf(&DEBUG_UART, "ID������ʧ��\r\n");
            return;
        }
        //usart_printf(&DEBUG_UART, "ID������\r\n");
        openDoorRC522(2000, ids, (uint8_t *) cardTypeStr);
        memset(cardTypeStr, 0, sizeof(cardTypeStr));

    }

}


/**
 *������֤NFC��������,������֤���
* @param pKey ����洢������
 * @param pSnr
 * @return
 */
uint8_t authStateNFC(const uint8_t pKey[]) {
    //��̬��ȡ��ά���鳤��
    uint16_t len = sizeof(PhoneNFC) / sizeof(PhoneNFC[0]);
    for (int i = 0; i < len; i++) {
        uint8_t c = 0;
        for (int j = 0; j < 4; ++j) {
            if (PhoneNFC[i][j] == pKey[j]) {
                if (++c >= 4) {
                    return 1;
                }
            }
        }
    }
    return 0;
}


/**
 *  ˢ��ʧ�ܼ�¼
 * @param id  ����
 * @param err   ������Ϣ
 */
void errorRFIDMQTT(uint8_t *id, uint8_t *err) {
    usart_printf_dma(&ESP_UART,
                     "%c{\"topic\":\"%s\",\"data\":{\"message\":\"%s\",\"status\":0,\"type\":\"%s\",\"id\":\"%s\"}}%c",
                     FRAME_HEAD, DEFAULT_PUB_TOPIC, err, TYPE_RFID, id, FRAME_TAIL);
    usart_printf(&DEBUG_UART,
                 "%c{\"topic\":\"%s\",\"data\":{\"message\":\"%s\",\"status\":0,\"type\":\"%s\",\"id\":\"%s\"}}%c",
                 FRAME_HEAD, DEFAULT_PUB_TOPIC, err, TYPE_RFID, id, FRAME_TAIL);
}

/**
 * ¼�뿨
 */
uint8_t entryCard(void) {
    uint8_t Card_KEY[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};    //{0x11,0x11,0x11,0x11,0x11,0x11};
    uint8_t status;  //״̬
    uint8_t Card_ID[4];
    uint8_t Card_Data[20];
    LCD_Fill(0, 100, lcddev.width, 160, BLACK);
    Show_Str_Mid(0, 100, (uint8_t *) "���ID��", 16, 240);

    if (MI_OK == PcdRequest(0x52, Card_Type))  //Ѱ������������ɹ����� MI_OK  ��ӡ��ο���
    {
        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
        status = PcdAnticoll(Card_ID);  //����ײ ����ɹ�����MI_OK
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "ID����ײ����\r\n");
            LCD_Fill(0, 100, lcddev.width, 160, BLACK);
            Show_Str_Mid(0, 100, (uint8_t *) "ID����ײ", 16, 240);
            return 0;
        }
        usart_printf(&DEBUG_UART, "ID���Ϊ:%02X %02X %02X %02X\r\n", Card_ID[0], Card_ID[1], Card_ID[2],
                     Card_ID[3]);//16����
        uint8_t ids[9];
        sprintf((char *) ids, "%02X%02X%02X%02X", Card_ID[0], Card_ID[1], Card_ID[2], Card_ID[3]);
        usart_printf(&DEBUG_UART, "IDΪ:%d %d %d %d \r\n", Card_ID[0], Card_ID[1], Card_ID[2], Card_ID[3]);//ʮ����
        status = PcdSelect(Card_ID);  //ѡ�� ����ɹ�����MI_OK
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "ѡ��ʧ��\r\n");
            return 0;
        }
        // ������֤
        status = PcdAuthState(PICC_AUTHENT1B, 2, Card_KEY, Card_ID);//��֤������ ����ɹ�����MI_OK
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "����������֤ʧ��\r\n");
        }
        //�޸�����
        status = changePassword(1, DEFAULT_PASSWORD);
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "�޸�����ʧ��\r\n");
            return 0;
        }
        usart_printf(&DEBUG_UART, "�޸�����ɹ�\r\n");
        PcdRead(2, Card_Data);
        usart_printf(&DEBUG_UART, "��ȡ:%s\r\n", Card_Data);

        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
        Show_Str_Mid(0, 100, (uint8_t *) "¼��ɹ�", 16, 240);
        delay_ms(1000);
        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
        //��Ƭ��������״̬
        status = PcdHalt();
        if (status != MI_OK) {
            return 0;
        }
        memset(cardTypeStr, 0, sizeof(cardTypeStr));
        return 1;
    }
}

/**
 * ɾ����
 * @return
 */
uint8_t delCard(void) {
    uint8_t Card_KEY[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};    //{0x11,0x11,0x11,0x11,0x11,0x11};
    uint8_t status;  //״̬
    uint8_t Card_ID[4];
    LCD_Fill(0, 100, lcddev.width, 160, BLACK);
    Show_Str_Mid(0, 100, (uint8_t *) "���ID��", 16, 240);
    if (MI_OK == PcdRequest(0x52, Card_Type))  //Ѱ������������ɹ����� MI_OK  ��ӡ��ο���
    {
        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
        status = PcdAnticoll(Card_ID);  //����ײ ����ɹ�����MI_OK
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "ID����ײ����\r\n");
            LCD_Fill(0, 100, lcddev.width, 160, BLACK);
            Show_Str_Mid(0, 100, (uint8_t *) "ID����ײ", 16, 240);
            return 0;
        }
        usart_printf(&DEBUG_UART, "ID���Ϊ:%02X %02X %02X %02X\r\n", Card_ID[0], Card_ID[1], Card_ID[2],
                     Card_ID[3]);//16����
        uint8_t ids[9];
        sprintf((char *) ids, "%02X%02X%02X%02X", Card_ID[0], Card_ID[1], Card_ID[2], Card_ID[3]);
        usart_printf(&DEBUG_UART, "IDΪ:%d %d %d %d \r\n", Card_ID[0], Card_ID[1], Card_ID[2], Card_ID[3]);//ʮ����
        status = PcdSelect(Card_ID);  //ѡ�� ����ɹ�����MI_OK
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "ѡ��ʧ��\r\n");
            return 0;
        }
        // ������֤
        status = PcdAuthState(PICC_AUTHENT1B, 2, Card_KEY, Card_ID);//��֤������ ����ɹ�����MI_OK
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "����������֤ʧ��\r\n");
        }
        //�޸�����
        status = changePassword(1, DELETE_PASSWORD);
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "�޸�����ʧ��\r\n");
            return 0;
        }
        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
        Show_Str_Mid(0, 100, (uint8_t *) "ɾ���ɹ�", 16, 240);
        delay_ms(1000);
        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
        //��Ƭ��������״̬
        status = PcdHalt();
        if (status != MI_OK) {
            return 0;
        }
        memset(cardTypeStr, 0, sizeof(cardTypeStr));
        return 1;
    }

}
