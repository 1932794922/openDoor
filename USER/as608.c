#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "usart.h"
#include "as608.h"
#include "usartMy.h"
#include "lcd.h"
#include "ui.h"
#include "AS608HARD.h"
#include "ESP8266.h"
#include "cJSON.h"
#include "funHandle.h"


/*���ڽ����жϴ�����stm32f1xx_it.c����*/
uint32_t AS608Addr = 0XFFFFFFFF;//Ĭ��

uint8_t USART3_RX_BUF[USART3_MAX_RECV_LEN];                //���ջ���,���USART3_MAX_RECV_LEN���ֽ�.

uint8_t Get_Device_Code[10] = {0x01, 0x00, 0x07, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b}; //������֤

uint8_t USART3_RX_STA = 0; //�����Ƿ���յ�����


SysPara AS608Para;//ָ��ģ��AS608����
uint16_t ValidN;

//���ڷ���һ���ֽ�
static uint8_t MYUSART_SendData(uint8_t data) {
    if (HAL_UART_Transmit(&AS608_UART, &data, 1, 0xff) == HAL_OK)
        return 0;

    return 1;
}

//���Ͱ�ͷ
static void SendHead(void) {
    memset(USART3_RX_BUF, 0, sizeof(USART3_RX_BUF)); //����ǰ������ݣ���Ϊ���ж�Ҫ���Ͱ�ͷ������ֻ��Ҫ�ڷ��Ͱ�ͷǰ��ռ���
    MYUSART_SendData(0xEF);
    MYUSART_SendData(0x01);
}

//���͵�ַ
static void SendAddr(void) {
    MYUSART_SendData(AS608Addr >> 24);
    MYUSART_SendData(AS608Addr >> 16);
    MYUSART_SendData(AS608Addr >> 8);
    MYUSART_SendData(AS608Addr);
}

//���Ͱ���ʶ,
static void SendFlag(uint8_t flag) {
    MYUSART_SendData(flag);
}

//���Ͱ�����
static void SendLength(int length) {
    MYUSART_SendData(length >> 8);
    MYUSART_SendData(length);
}

//����ָ����
static void Sendcmd(uint8_t cmd) {
    MYUSART_SendData(cmd);
}

//����У���
static void SendCheck(uint16_t check) {
    MYUSART_SendData(check >> 8);
    MYUSART_SendData(check);
}

/*****************************************
��������uint8_t AS608_Check(void)
��������
����������ģ���Ƿ����Ӽ��
����ֵ��ģ�������˷���0 ���򷵻�1
*****************************************/
static uint8_t AS608_Check(void) {
    USART3_RX_BUF[9] = 1;

    SendHead();
    SendAddr();

    for (int i = 0; i < 10; i++) {
        MYUSART_SendData(Get_Device_Code[i]);
    }

    //HAL_UART_Receive(&AS608_UART,USART3_RX_BUF,12,100);//����������12������
    delay_ms(200);//�ȴ�200ms

    if (USART3_RX_BUF[9] == 0)
        return 0;

    return 1;
}

/*ָ��ģ���ʼ��*/
uint8_t as608_init(void) {
    //����uart3�����ж�
    HAL_UART_Receive_IT(&AS608_UART, USART3_RX_BUF, sizeof(USART3_RX_BUF)); //�������ݣ��Ҳ����ж�
    //ʹ�ܿ����ж�
    __HAL_UART_ENABLE_IT(&AS608_UART, UART_IT_IDLE); //

    return AS608_Check();
}

//�ж��жϽ��յ�������û��Ӧ���
//waittimeΪ�ȴ��жϽ������ݵ�ʱ�䣨��λ1ms��
//����ֵ�����ݰ��׵�ַ
static uint8_t *JudgeStr(uint16_t waittime) {
    char *data;
    uint8_t str[8];
    str[0] = 0xef;
    str[1] = 0x01;
    str[2] = AS608Addr >> 24;
    str[3] = AS608Addr >> 16;
    str[4] = AS608Addr >> 8;
    str[5] = AS608Addr;
    str[6] = 0x07;
    str[7] = '\0';
    USART3_RX_STA = 0;

    while (--waittime) {
        delay_ms(1);

        if (USART3_RX_STA) //���յ�һ������
        {
            USART3_RX_STA = 0;
            data = strstr((const char *) USART3_RX_BUF, (const char *) str);

            if (data)
                return (uint8_t *) data;
        }
    }

    return 0;
}

//¼��ͼ�� PS_GetImage
//����:̽����ָ��̽�⵽��¼��ָ��ͼ�����ImageBuffer��
//ģ�鷵��ȷ����
uint8_t PS_GetImage(void) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//�������ʶ
    SendLength(0x03);
    Sendcmd(0x01);
    temp = 0x01 + 0x03 + 0x01;
    SendCheck(temp);
    data = JudgeStr(2000);

    if (data)
        ensure = data[9];
    else
        ensure = 0xff;

    return ensure;
}

//�������� PS_GenChar
//����:��ImageBuffer�е�ԭʼͼ������ָ�������ļ�����CharBuffer1��CharBuffer2
//����:BufferID --> charBuffer1:0x01	charBuffer1:0x02
//ģ�鷵��ȷ����
uint8_t PS_GenChar(uint8_t BufferID) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//�������ʶ
    SendLength(0x04);
    Sendcmd(0x02);
    MYUSART_SendData(BufferID);
    temp = 0x01 + 0x04 + 0x02 + BufferID;
    SendCheck(temp);
    data = JudgeStr(2000);

    if (data)
        ensure = data[9];
    else
        ensure = 0xff;

    return ensure;
}

//��ȷ�ȶ���öָ������ PS_Match
//����:��ȷ�ȶ�CharBuffer1 ��CharBuffer2 �е������ļ�
//ģ�鷵��ȷ����
uint8_t PS_Match(void) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//�������ʶ
    SendLength(0x03);
    Sendcmd(0x03);
    temp = 0x01 + 0x03 + 0x03;
    SendCheck(temp);
    data = JudgeStr(2000);

    if (data)
        ensure = data[9];
    else
        ensure = 0xff;

    return ensure;
}

//����ָ�� PS_Search
//����:��CharBuffer1��CharBuffer2�е������ļ����������򲿷�ָ�ƿ�.�����������򷵻�ҳ�롣
//����:  BufferID @ref CharBuffer1	CharBuffer2
//˵��:  ģ�鷵��ȷ���֣�ҳ�루����ָ��ģ�壩
uint8_t PS_Search(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, SearchResult *p) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//�������ʶ
    SendLength(0x08);
    Sendcmd(0x04);
    MYUSART_SendData(BufferID);
    MYUSART_SendData(StartPage >> 8);
    MYUSART_SendData(StartPage);
    MYUSART_SendData(PageNum >> 8);
    MYUSART_SendData(PageNum);
    temp = 0x01 + 0x08 + 0x04 + BufferID
           + (StartPage >> 8) + (uint8_t) StartPage
           + (PageNum >> 8) + (uint8_t) PageNum;
    SendCheck(temp);
    data = JudgeStr(2000);

    if (data) {
        ensure = data[9];
        p->pageID = (data[10] << 8) + data[11];
        p->mathscore = (data[12] << 8) + data[13];
    } else
        ensure = 0xff;

    return ensure;
}

//�ϲ�����������ģ�壩PS_RegModel
//����:��CharBuffer1��CharBuffer2�е������ļ��ϲ����� ģ��,�������CharBuffer1��CharBuffer2
//˵��:  ģ�鷵��ȷ����
uint8_t PS_RegModel(void) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//�������ʶ
    SendLength(0x03);
    Sendcmd(0x05);
    temp = 0x01 + 0x03 + 0x05;
    SendCheck(temp);
    data = JudgeStr(2000);

    if (data)
        ensure = data[9];
    else
        ensure = 0xff;

    return ensure;
}

//����ģ�� PS_StoreChar
//����:�� CharBuffer1 �� CharBuffer2 �е�ģ���ļ��浽 PageID ��flash���ݿ�λ�á�
//����:  BufferID @ref charBuffer1:0x01	charBuffer1:0x02
//       PageID��ָ�ƿ�λ�úţ�
//˵��:  ģ�鷵��ȷ����
uint8_t PS_StoreChar(uint8_t BufferID, uint16_t PageID) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//�������ʶ
    SendLength(0x06);
    Sendcmd(0x06);
    MYUSART_SendData(BufferID);
    MYUSART_SendData(PageID >> 8);
    MYUSART_SendData(PageID);
    temp = 0x01 + 0x06 + 0x06 + BufferID
           + (PageID >> 8) + (uint8_t) PageID;
    SendCheck(temp);
    data = JudgeStr(2000);

    if (data)
        ensure = data[9];
    else
        ensure = 0xff;

    return ensure;
}

//ɾ��ģ�� PS_DeletChar
//����:  ɾ��flash���ݿ���ָ��ID�ſ�ʼ��N��ָ��ģ��
//����:  PageID(ָ�ƿ�ģ���)��Nɾ����ģ�������
//˵��:  ģ�鷵��ȷ����
uint8_t PS_DeletChar(uint16_t PageID, uint16_t N) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//�������ʶ
    SendLength(0x07);
    Sendcmd(0x0C);
    MYUSART_SendData(PageID >> 8);
    MYUSART_SendData(PageID);
    MYUSART_SendData(N >> 8);
    MYUSART_SendData(N);
    temp = 0x01 + 0x07 + 0x0C
           + (PageID >> 8) + (uint8_t) PageID
           + (N >> 8) + (uint8_t) N;
    SendCheck(temp);
    data = JudgeStr(2000);

    if (data)
        ensure = data[9];
    else
        ensure = 0xff;

    return ensure;
}

//���ָ�ƿ� PS_Empty
//����:  ɾ��flash���ݿ�������ָ��ģ��
//����:  ��
//˵��:  ģ�鷵��ȷ����
uint8_t PS_Empty(void) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//�������ʶ
    SendLength(0x03);
    Sendcmd(0x0D);
    temp = 0x01 + 0x03 + 0x0D;
    SendCheck(temp);
    data = JudgeStr(2000);

    if (data)
        ensure = data[9];
    else
        ensure = 0xff;

    return ensure;
}

//дϵͳ�Ĵ��� PS_WriteReg
//����:  дģ��Ĵ���
//����:  �Ĵ������RegNum:4\5\6
//˵��:  ģ�鷵��ȷ����
uint8_t PS_WriteReg(uint8_t RegNum, uint8_t DATA) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//�������ʶ
    SendLength(0x05);
    Sendcmd(0x0E);
    MYUSART_SendData(RegNum);
    MYUSART_SendData(DATA);
    temp = RegNum + DATA + 0x01 + 0x05 + 0x0E;
    SendCheck(temp);
    data = JudgeStr(2000);

    if (data)
        ensure = data[9];
    else
        ensure = 0xff;

    if (ensure == 0)
        usart_printf(&DEBUG_UART, "\r\n���ò����ɹ���");
    else
        usart_printf(&DEBUG_UART, "\r\n%s", EnsureMessage(ensure));

    return ensure;
}

//��ϵͳ�������� PS_ReadSysPara
//����:  ��ȡģ��Ļ��������������ʣ�����С��)
//����:  ��
//˵��:  ģ�鷵��ȷ���� + ����������16bytes��
uint8_t PS_ReadSysPara(SysPara *p) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//�������ʶ
    SendLength(0x03);
    Sendcmd(0x0F);
    temp = 0x01 + 0x03 + 0x0F;
    SendCheck(temp);
    data = JudgeStr(1000);

    if (data) {
        ensure = data[9];
        p->PS_max = (data[14] << 8) + data[15];
        p->PS_level = data[17];
        p->PS_addr = (data[18] << 24) + (data[19] << 16) + (data[20] << 8) + data[21];
        p->PS_size = data[23];
        p->PS_N = data[25];
    } else
        ensure = 0xff;

//    if(ensure == 0x00)
//    {
//        usart_printf(&DEBUG_UART, "\r\nģ�����ָ������=%d", p->PS_max);
//        usart_printf(&DEBUG_UART, "\r\n�Աȵȼ�=%d", p->PS_level);
//        usart_printf(&DEBUG_UART, "\r\n��ַ=%x", p->PS_addr);
//        usart_printf(&DEBUG_UART, "\r\n������=%d", p->PS_N * 9600);
//    }
//    else
//        usart_printf(&DEBUG_UART, "\r\n%s", EnsureMessage(ensure));

    return ensure;
}

//����ģ���ַ PS_SetAddr
//����:  ����ģ���ַ
//����:  PS_addr
//˵��:  ģ�鷵��ȷ����
uint8_t PS_SetAddr(uint32_t PS_addr) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//�������ʶ
    SendLength(0x07);
    Sendcmd(0x15);
    MYUSART_SendData(PS_addr >> 24);
    MYUSART_SendData(PS_addr >> 16);
    MYUSART_SendData(PS_addr >> 8);
    MYUSART_SendData(PS_addr);
    temp = 0x01 + 0x07 + 0x15
           + (uint8_t) (PS_addr >> 24) + (uint8_t) (PS_addr >> 16)
           + (uint8_t) (PS_addr >> 8) + (uint8_t) PS_addr;
    SendCheck(temp);
    AS608Addr = PS_addr; //������ָ�������ַ
    data = JudgeStr(2000);

    if (data)
        ensure = data[9];
    else
        ensure = 0xff;

    AS608Addr = PS_addr;

    if (ensure == 0x00)
        usart_printf(&DEBUG_UART, "\r\n���õ�ַ�ɹ���");
    else
        usart_printf(&DEBUG_UART, "\r\n%s", EnsureMessage(ensure));

    return ensure;
}

//���ܣ� ģ���ڲ�Ϊ�û�������256bytes��FLASH�ռ����ڴ��û����±�,
//	�ü��±��߼��ϱ��ֳ� 16 ��ҳ��
//����:  NotePageNum(0~15),Byte32(Ҫд�����ݣ�32���ֽ�)
//˵��:  ģ�鷵��ȷ����
uint8_t PS_WriteNotepad(uint8_t NotePageNum, uint8_t *Byte32) {
    uint16_t temp;
    uint8_t ensure, i;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//�������ʶ
    SendLength(36);
    Sendcmd(0x18);
    MYUSART_SendData(NotePageNum);

    for (i = 0; i < 32; i++) {
        MYUSART_SendData(Byte32[i]);
        temp += Byte32[i];
    }

    temp = 0x01 + 36 + 0x18 + NotePageNum + temp;
    SendCheck(temp);
    data = JudgeStr(2000);

    if (data)
        ensure = data[9];
    else
        ensure = 0xff;

    return ensure;
}

//������PS_ReadNotepad
//���ܣ�  ��ȡFLASH�û�����128bytes����
//����:  NotePageNum(0~15)
//˵��:  ģ�鷵��ȷ����+�û���Ϣ
uint8_t PS_ReadNotepad(uint8_t NotePageNum, uint8_t *Byte32) {
    uint16_t temp;
    uint8_t ensure, i;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//�������ʶ
    SendLength(0x04);
    Sendcmd(0x19);
    MYUSART_SendData(NotePageNum);
    temp = 0x01 + 0x04 + 0x19 + NotePageNum;
    SendCheck(temp);
    data = JudgeStr(2000);

    if (data) {
        ensure = data[9];

        for (i = 0; i < 32; i++) {
            Byte32[i] = data[10 + i];
        }
    } else
        ensure = 0xff;

    return ensure;
}

//��������PS_HighSpeedSearch
//���ܣ��� CharBuffer1��CharBuffer2�е������ļ��������������򲿷�ָ�ƿ⡣
//		  �����������򷵻�ҳ��,��ָ����ڵ�ȷ������ָ�ƿ��� ���ҵ�¼ʱ����
//		  �ܺõ�ָ�ƣ���ܿ�������������
//����:  BufferID�� StartPage(��ʼҳ)��PageNum��ҳ����
//˵��:  ģ�鷵��ȷ����+ҳ�루����ָ��ģ�壩
uint8_t PS_HighSpeedSearch(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, SearchResult *p) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//�������ʶ
    SendLength(0x08);
    Sendcmd(0x1b);
    MYUSART_SendData(BufferID);
    MYUSART_SendData(StartPage >> 8);
    MYUSART_SendData(StartPage);
    MYUSART_SendData(PageNum >> 8);
    MYUSART_SendData(PageNum);
    temp = 0x01 + 0x08 + 0x1b + BufferID
           + (StartPage >> 8) + (uint8_t) StartPage
           + (PageNum >> 8) + (uint8_t) PageNum;
    SendCheck(temp);
    data = JudgeStr(2000);

    if (data) {
        ensure = data[9];
        p->pageID = (data[10] << 8) + data[11];
        p->mathscore = (data[12] << 8) + data[13];
    } else
        ensure = 0xff;

    return ensure;
}

//����Чģ����� PS_ValidTempleteNum
//���ܣ�����Чģ�����
//����: ��
//˵��: ģ�鷵��ȷ����+��Чģ�����ValidN
uint8_t PS_ValidTempleteNum(uint16_t *ValidN) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//�������ʶ
    SendLength(0x03);
    Sendcmd(0x1d);
    temp = 0x01 + 0x03 + 0x1d;
    SendCheck(temp);
    data = JudgeStr(2000);

    if (data) {
        ensure = data[9];
        *ValidN = (data[10] << 8) + data[11];
    } else
        ensure = 0xff;

//    if(ensure == 0x00)
//    {
//        usart_printf(&DEBUG_UART, "\r\n��Чָ�Ƹ���=%d", (data[10] << 8) + data[11]);
//    }
//    else
//        usart_printf(&DEBUG_UART, "\r\n%s", EnsureMessage(ensure));

    return ensure;
}

//��AS608���� PS_HandShake
//����: PS_Addr��ַָ��
//˵��: ģ�鷵�µ�ַ����ȷ��ַ��
uint8_t PS_HandShake(uint32_t *PS_Addr) {
    SendHead();
    SendAddr();
    MYUSART_SendData(0X01);
    MYUSART_SendData(0X00);
    MYUSART_SendData(0X00);
    delay_ms(200);

    if (USART3_RX_STA) //���յ�����
    {
        if (//�ж��ǲ���ģ�鷵�ص�Ӧ���
                USART3_RX_BUF[0] == 0XEF
                && USART3_RX_BUF[1] == 0X01
                && USART3_RX_BUF[6] == 0X07
                ) {
            *PS_Addr = (USART3_RX_BUF[2] << 24) + (USART3_RX_BUF[3] << 16)
                       + (USART3_RX_BUF[4] << 8) + (USART3_RX_BUF[5]);
            USART3_RX_STA = 0;
            return 0;
        }

        USART3_RX_STA = 0;
    }

    return 1;
}

//ģ��Ӧ���ȷ������Ϣ����
//���ܣ�����ȷ���������Ϣ������Ϣ
//����: ensure
const char *EnsureMessage(uint8_t ensure) {
    const static char *p;

    switch (ensure) {
        case 0x00:
            p = "OK";
            break;

        case 0x01:
            p = " ���ݰ����մ��� ";
            break;

        case 0x02:
            p = "�밴ѹ������";
            break;

        case 0x03:
            p = "¼��ָ��ͼ��ʧ��";
            break;

        case 0x04:
            p = "ָ��̫�ɻ�̫��";
            break;

        case 0x05:
            p = "ָ��̫ʪ��̫��";
            break;

        case 0x06:
            p = "ָ��ͼ��̫��";
            break;

        case 0x07:
            p = "ָ��������̫��";
            break;

        case 0x08:
            p = "ָ�Ʋ�ƥ��";
            break;

        case 0x09:
            p = "no prints found";
            break;

        case 0x0a:
            p = "feature merge failed";
            break;

        case 0x0b:
            p = "Address serial number out of range";

        case 0x10:
            p = "Failed to delete template";
            break;

        case 0x11:
            p = "Failed to clear the fingerprint library";
            break;

        case 0x15:
            p = "no valid image in buffer";
            break;

        case 0x18:
            p = "Error reading and writing FLASH";
            break;

        case 0x19:
            p = "undefined error";
            break;

        case 0x1a:
            p = "invalid register number";
            break;

        case 0x1b:
            p = "Register content error";
            break;

        case 0x1c:
            p = "notepad page number error";
            break;

        case 0x1f:
            p = "fingerprint library full";
            break;

        case 0x20:
            p = "wrong address";
            break;

        default :
            p = "Return confirmation code is wrong";
            break;
    }

    return p;
}

//��ʾȷ���������Ϣ
void ShowErrMessage(uint8_t ensure) {
    LCD_Fill(0, 100, lcddev.width, 160, BLACK);
    LCD_ShowString(80, 100, (uint8_t *) EnsureMessage(ensure), RED, BLACK); //80,100
    usart_printf(&DEBUG_UART, "%s\r\n", EnsureMessage(ensure));
    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "type", TYPE_FINGERTIP);
    cJSON_AddStringToObject(data, "message", EnsureMessage(ensure));
    cJSON_AddNumberToObject(data, "status", 0);
    char *json_string = cJSON_PrintUnformatted(data);
    sendMQTTHaveTopic(DEFAULT_PUB_TOPIC, json_string);
    free(json_string);
    cJSON_Delete(data);
    alarmError();

}





//¼ָ��

void Add_FR(void) {
    uint8_t i = 0, ensure, processnum = 0;
    uint16_t ID_NUM = 0;
    while (1) {
        switch (processnum) {
            case 0:
                i++;
                //usart_printf(&DEBUG_UART, "�밴¼��ָ��\r\n");
                LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                Show_Str_Mid(0, 100, (uint8_t *) "�밴¼��ָ��", 16, 240);
                ensure = PS_GetImage();
                if (ensure == 0x00) {
                    ensure = PS_GenChar(CharBuffer1); //��������
                    if (ensure == 0x00) {
                        //usart_printf(&DEBUG_UART, "ָ������\r\n");
                        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                        Show_Str_Mid(0, 100, (uint8_t *) "ָ������", 16, 240);
                        i = 0;
                        processnum = 1; //�����ڶ���
                    } else ShowErrMessage(ensure);
                } else {
                    delay_ms(300);
                    if (ensure != 0x02) {
                        ShowErrMessage(ensure);

                    }
                }
                break;

            case 1:
                i++;
                // usart_printf(&DEBUG_UART, "���ٰ�һ��\r\n");
                LCD_Fill(0, 120, lcddev.width, 160, BLACK);
                Show_Str_Mid(0, 120, (uint8_t *) "���ٰ�һ��", 16, 240);
                ensure = PS_GetImage();
                delay_ms(500);
                if (ensure == 0x00) {
                    ensure = PS_GenChar(CharBuffer2); //��������
                    if (ensure == 0x00) {
                        // usart_printf(&DEBUG_UART, "ָ������\r\n");
                        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                        LCD_Fill(0, 120, lcddev.width, 160, BLACK);
                        Show_Str_Mid(0, 100, (uint8_t *) "ָ������", 16, 240);
                        i = 0;
                        processnum = 2; //����������
                    } else ShowErrMessage(ensure);
                } else {
                    if (ensure != 0x02) {
                        ShowErrMessage(ensure);

                    }
                }
                break;
            case 2:
                // usart_printf(&DEBUG_UART, "�Ա�����ָ��\r\n");
                LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                Show_Str_Mid(0, 100, (uint8_t *) "�Ա�����ָ��", 16, 240);
                ensure = PS_Match();
                delay_ms(500);
                if (ensure == 0x00) {
                    // usart_printf(&DEBUG_UART, "�Աȳɹ�\r\n");
                    LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                    Show_Str_Mid(0, 100, (uint8_t *) "�Աȳɹ�", 16, 240);
                    processnum = 3; //�������Ĳ�
                } else {
                    // usart_printf(&DEBUG_UART, "�Ա�ʧ��\r\n");
                    LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                    Show_Str_Mid(0, 100, (uint8_t *) "�Ա�ʧ��", 16, 240);
                    ShowErrMessage(ensure);
                    i = 0;
                    processnum = 0; //���ص�һ��
                }

                delay_ms(500);
                break;

            case 3:
                //usart_printf(&DEBUG_UART, "����ָ��ģ��\r\n");
                LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                Show_Str_Mid(0, 100, (uint8_t *) "����ָ��ģ��", 16, 240);
                delay_ms(500);
                ensure = PS_RegModel();

                if (ensure == 0x00) {
                    // usart_printf(&DEBUG_UART, "����ָ��ģ��ɹ�\r\n");
                    LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                    Show_Str_Mid(0, 100, (uint8_t *) "����ָ��ģ��ɹ�", 16, 240);
                    processnum = 4; //�������岽
                } else {
                    processnum = 0;
                    ShowErrMessage(ensure);
                }

                delay_ms(1000);
                break;

            case 4:
                LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                Show_Str_Mid(0, 100, (uint8_t *) "�����봢��ID,��Enter����", 16, 240);
                Show_Str_Mid(0, 120, (uint8_t *) "0=< ID <=299", 16, 240);
                do {
                    ID_NUM = GET_NUM();
                } while (ID_NUM >= AS608Para.PS_max);        //����ID����С��ָ�����������ֵ
                ensure = PS_StoreChar(CharBuffer2, ID_NUM); //����ģ��
                if (ensure == 0x00) {
                    uint16_t ValidN;
                    LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                    uint8_t str[32];
                    sprintf((char *) str, "¼��ָ�Ƴɹ� ID: %d", ID_NUM);
                    Show_Str_Mid(0, 100, (uint8_t *) str, 16, 240);
                    PS_ValidTempleteNum(&ValidN); //����ָ�Ƹ���
                    LCD_ShowNum(64, 60, AS608Para.PS_max - ValidN, 3, 16, RED, BLACK);
                    delay_ms(1500);
                    LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                    LCD_Fill(0, 120, lcddev.width, 160, BLACK);
                    alarmSuccess();
                    return;
                } else {
                    processnum = 0;
                    ShowErrMessage(ensure);
                }

                break;
        }

        delay_ms(800);

        if (i >= 5) //����5��û�а���ָ���˳�
        {
            LCD_Fill(0, 100, lcddev.width, 160, BLACK);
            break;
        }
    }
}


//ˢָ��
uint8_t ensure;

void press_FR(void) {
    SearchResult seach;

    ensure = PS_GetImage();

    if (ensure == 0x00) //��ȡͼ��ɹ�
    {
        ensure = PS_GenChar(CharBuffer1);

        if (ensure == 0x00) //���������ɹ�
        {
            ensure = PS_HighSpeedSearch(CharBuffer1, 0, 99, &seach);

            if (ensure == 0x00) //�����ɹ�
            {
                //LCD_Clear(BLACK);//���� LIGHTGREEN
                LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                uint8_t str[32];
                sprintf((char *) str, "��֤�ɹ�:ID:%d", seach.pageID);
                //usart_printf(&DEBUG_UART, "ָ����֤�ɹ�:  ID:%d �÷�:%d\r\n ", seach.pageID, seach.mathscore);
                Show_Str_Mid(0, 100, (uint8_t *) str, 16, 240);
                //���͵�MQTT������ json�ַ���
                cJSON *data = cJSON_CreateObject();
                cJSON_AddStringToObject(data, "type", TYPE_FINGERTIP);
                cJSON_AddNumberToObject(data, "id", seach.pageID);
                cJSON_AddNumberToObject(data, "score", seach.mathscore);
                cJSON_AddNumberToObject(data, "status", 1);
                cJSON_AddStringToObject(data, "message", "open the door");
                char *json_string = cJSON_PrintUnformatted(data);
                sendMQTTHaveTopic(DEFAULT_PUB_TOPIC, json_string);
                openDoorMG995();
                free(json_string);
                cJSON_Delete(data);
            } else {
                ShowErrMessage(ensure);
                usart_printf(&DEBUG_UART, "��֤ʧ��\r\n");
                delay_ms(1500);
            }
        } else {
            ShowErrMessage(ensure);
        };

        //usart_printf(&DEBUG_UART, "�밴ָ��\r\n");
        delay_ms(1500);
        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
        Show_Str_Mid(0, 100, (uint8_t *) "�밴ָ��", 16, 240);
    }

}

//ɾ������ָ��
void Del_FR(void) {
    uint8_t str[32];
    uint8_t ensure;
    uint16_t ID_NUM = 0;
    LCD_Fill(0, 80, lcddev.width, 160, BLACK);
    Show_Str_Mid(0, 120, (uint8_t *) "������ָ��ID,��Enter����", 16, 240);
    Show_Str_Mid(0, 140, (uint8_t *) "0=< ID <=299", 16, 240);
    delay_ms(50);
    AS608_load_keyboard(0, 170, (uint8_t **) kbd_delFR);
    ID_NUM = GET_NUM(); //��ȡ���ص���ֵ.

    if (ID_NUM == 0xFFFF)
        goto MENU; //������ҳ��
    else if (ID_NUM == 0xFF00)
        ensure = PS_Empty(); //���ָ�ƿ�
    else
        ensure = PS_DeletChar(ID_NUM, 1); //ɾ������ָ��

    if (ensure == 0 && ID_NUM == 0xFF00) {
        LCD_Fill(0, 80, lcddev.width, 160, BLACK);
        sprintf((char *) str, "���ָ�ƿ�ɹ�");
        Show_Str_Mid(0, 140, (uint8_t *) str, 16, 240);
        usart_printf(&DEBUG_UART, "ɾ��ָ�Ƴɹ� \r\n");
        alarmSuccess();
    } else if (ensure == 0) {
        sprintf((char *) str, "ɾ��ָ�Ƴɹ�ID: %d", ID_NUM);
        LCD_Fill(0, 80, lcddev.width, 160, BLACK);
        Show_Str_Mid(0, 140, (uint8_t *) str, 16, 240);
        usart_printf(&DEBUG_UART, "ɾ��ָ�Ƴɹ� \r\n");
        alarmSuccess();
    } else {
        ShowErrMessage(ensure);

    }

    delay_ms(1200);
    PS_ValidTempleteNum(&ValidN); //����ָ�Ƹ���
    LCD_ShowNum(64, 60, AS608Para.PS_max - ValidN, 3, 16, RED, BLACK);

    MENU:
    LCD_Fill(0, 80, lcddev.width, 160, BLACK);
    delay_ms(50);
    AS608_load_keyboard(0, 170, (uint8_t **) kbd_menu);

}

/*���ָ�ƿ�*/
void Del_FR_Lib(void) {
    uint8_t ensure;
    usart_printf(&DEBUG_UART, "ɾ��ָ�ƿ⿪ʼ\r\n");
    ensure = PS_Empty(); //���ָ�ƿ�

    if (ensure == 0) {
        usart_printf(&DEBUG_UART, "���ָ�ƿ�ɹ�\r\n");
    } else
        ShowErrMessage(ensure);

    delay_ms(1500);
}

