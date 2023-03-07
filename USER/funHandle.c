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
}

uint8_t Card_KEY[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};    //{0x11,0x11,0x11,0x11,0x11,0x11};


uint8_t DEFAULT_PASSWORD[16] = {    /*!< ����ÿ������������д*/
        0x04, 0x05, 0x02, 0x07, 0x02, 0x05,
        0xff, 0x07, 0x80, 0x69,
        0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
};

uint8_t PhoneNFC[][16] = {
        {1, 69, 184, 39},
        {40, 235, 32, 60}
};


void Read_IDcard(void) {
    uint8_t status;  //״̬
    uint8_t Card_ID[4];
    uint8_t Card_Type1[2];
    uint8_t Card_Data[20];
    if (MI_OK == PcdRequest(0x52, Card_Type1))  //Ѱ������������ɹ����� MI_OK  ��ӡ��ο���
    {
        uint16_t cardType = (Card_Type1[0] << 8) | Card_Type1[1];//����ͬ��������
        usart_printf(&DEBUG_UART, "�����ͣ�(0x%04X)\r\n", cardType);  //"Card Type(0x%04X):"
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
                usart_printf(&DEBUG_UART, "δ֪������\r\n");
                break;
        }
        status = PcdAnticoll(Card_ID);  //����ײ ����ɹ�����MI_OK
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "ID����ײ����\r\n");
            return;
        }
        usart_printf(&DEBUG_UART, "ID���Ϊ:%02X %02X %02X %02X\r\n", Card_ID[0], Card_ID[1], Card_ID[2],
                     Card_ID[3]);//16����
        usart_printf(&DEBUG_UART, "IDΪ:%d %d %d %d \r\n", Card_ID[0], Card_ID[1], Card_ID[2], Card_ID[3]);//ʮ����
        //sprintf(OK, "ID:%02X%02X%02X%02X", Card_ID[0], Card_ID[1], Card_ID[2], Card_ID[3]);

        status = PcdSelect(Card_ID);  //ѡ�� ����ɹ�����MI_OK
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "ѡ��ʧ��\r\n");
            return;
        }
        // ������֤
        status = PcdAuthState(PICC_AUTHENT1B, 2, Card_KEY, Card_ID);//��֤������ ����ɹ�����MI_OK
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "����������֤ʧ��\r\n");
            usart_printf(&DEBUG_UART, "��ʼ����ID���ж�\r\n");
            goto CHECKIDS; //������ҳ��
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
            usart_printf(&DEBUG_UART, "��ȡ����2��������ʧ��\r\n");
            return;
        }
        // �Ƚ�ǰ6���ֽ��Ƿ���ͬ
        if (memcmp(Card_Data, DEFAULT_PASSWORD, 6) != 0) {
            usart_printf(&DEBUG_UART, "������֤ʧ��\r\n");
            return;
        }
        usart_printf(&DEBUG_UART, "������֤�ɹ�\r\n");
#endif

        //��Ƭ��������״̬
        status = PcdHalt();
        if (status != MI_OK) {
            usart_printf(&DEBUG_UART, "ID������ʧ��\r\n");
            return;
        }
        usart_printf(&DEBUG_UART, "ID������\r\n");

        CHECKIDS:
        if (!authStateNFC(Card_ID)) {
            return;
        }
        usart_printf(&DEBUG_UART, "ͨ����֤ID�ųɹ�\r\n");
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
            //�û�Ϊ10����
            if (PhoneNFC[i][j] == pKey[j]) {
                if (++c >= 4) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
