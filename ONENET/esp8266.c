//��Ƭ��ͷ�ļ�
#include "main.h"
#include "usartMy.h"
//�����豸����
#include "esp8266.h"

//Ӳ������
#include "delay.h"
#include "usart.h"

//C��
#include <string.h>
#include <stdio.h>


#define ESP8266_WIFI_INFO		"AT+CWJAP=\"Trister\",\"13877899818.\"\r\n"

//#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"47.108.87.206\",1883,60\r\n"
#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"47.108.87.206\",1883\r\n"


unsigned char esp8266_buf[128];

unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;


//==========================================================
//	�������ƣ�	ESP8266_Clear
//
//	�������ܣ�	��ջ���
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����
//==========================================================
void ESP8266_Clear(void)
{
    memset(esp8266_buf, 0, sizeof(esp8266_buf));
    esp8266_cnt = 0;
}

//==========================================================
//	�������ƣ�	ESP8266_WaitRecive
//
//	�������ܣ�	�ȴ��������
//
//	��ڲ�����	��
//
//	���ز�����	REV_OK-�������		REV_WAIT-���ճ�ʱδ���
//
//	˵����		ѭ�����ü���Ƿ�������
//==========================================================
_Bool ESP8266_WaitRecive(void)
{



    if(esp8266_cnt == 0) 							//������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
        return REV_WAIT;

    if(esp8266_cnt == esp8266_cntPre)				//�����һ�ε�ֵ�������ͬ����˵���������
    {
        esp8266_cnt = 0;							//��0���ռ���

        return REV_OK;								//���ؽ�����ɱ�־
    }

    esp8266_cntPre = esp8266_cnt;					//��Ϊ��ͬ

    return REV_WAIT;								//���ؽ���δ��ɱ�־

}

//==========================================================
//	�������ƣ�	ESP8266_SendCmd
//
//	�������ܣ�	��������
//
//	��ڲ�����	cmd������
//				res����Ҫ���ķ���ָ��
//
//	���ز�����	0-�ɹ�	1-ʧ��
//
//	˵����
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res)
{

    unsigned char timeOut = 200;

    usart_printf(&huart3, cmd, strlen((const char *)cmd));
    delay_ms(200);
    usart_printf(&huart1, "%s\r\n", esp8266_buf);
    while(timeOut--)
    {
        if(ESP8266_WaitRecive() == REV_OK)	  //����յ�����
        {
            
            if(strstr((const char *)esp8266_buf, res) != NULL)		//����������ؼ���
            {
                ESP8266_Clear();									//��ջ���
                return 0;
            }
        }

        delay_ms(500);
    }

    return 1;

}

//==========================================================
//	�������ƣ�	ESP8266_SendData
//
//	�������ܣ�	��������
//
//	��ڲ�����	data������
//				len������
//
//	���ز�����	��
//
//	˵����
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{

    char cmdBuf[32];

    ESP8266_Clear();								//��ս��ջ���
    sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);		//��������

    if(!ESP8266_SendCmd(cmdBuf, ">"))				//�յ���>��ʱ���Է�������
    {
        usart_printf(&huart3, (const char *) data, len);		//�����豸������������
    }

}

//==========================================================
//	�������ƣ�	ESP8266_GetIPD
//
//	�������ܣ�	��ȡƽ̨���ص�����
//
//	��ڲ�����	�ȴ���ʱ��(����10ms)
//
//	���ز�����	ƽ̨���ص�ԭʼ����
//
//	˵����		��ͬ�����豸���صĸ�ʽ��ͬ����Ҫȥ����
//				��ESP8266�ķ��ظ�ʽΪ	"+IPD,x:yyy"	x�������ݳ��ȣ�yyy����������
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{

    char *ptrIPD = NULL;
    usart_printf(&huart1, "found %s \r\n",timeOut);
    do
    {
        if(ESP8266_WaitRecive() == REV_OK)								//����������
        {
            ptrIPD = strstr((char *)esp8266_buf, "IPD,");				//������IPD��ͷ

            if(ptrIPD == NULL)											//���û�ҵ���������IPDͷ���ӳ٣�������Ҫ�ȴ�һ�ᣬ�����ᳬ���趨��ʱ��
            {
                 usart_printf(&huart1, "\"IPD\" not found\r\n");
            }
            else
            {
                ptrIPD = strchr(ptrIPD, ':');							//�ҵ�':'

                if(ptrIPD != NULL)
                {
                    ptrIPD++;
                    return (unsigned char *)(ptrIPD);
                }
                else
                    return NULL;

            }
        }

        delay_ms(5);													//��ʱ�ȴ�
    }
    while(timeOut--);

    return NULL;														//��ʱ��δ�ҵ������ؿ�ָ��

}

//==========================================================
//	�������ƣ�	ESP8266_Init
//
//	�������ܣ�	��ʼ��ESP8266
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����
//==========================================================
void ESP8266_Init(void)
{

    ESP8266_Clear();

    usart_printf(&huart1, "0.AT\r\n");

    while(ESP8266_SendCmd("AT\r\n", "OK"))
        delay_ms(500);

    usart_printf(&huart1, "1.RST\r\n");

    ESP8266_SendCmd("AT+RST\r\n", "");
    delay_ms(500);
    
    usart_printf(&huart1, "CIPCLOSE\r\n");
    ESP8266_SendCmd("AT+CIPCLOSE\r\n", "");
    delay_ms(500);

    usart_printf(&huart1, "2. CWMODE\r\n");

    while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"))
        delay_ms(500);

    usart_printf(&huart1, "3. AT+CWDHCP\r\n");

    while(ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK"))
        delay_ms(500);

    usart_printf(&huart1, "4. CWJAP\r\n");

    while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP"))
        delay_ms(1000);

    usart_printf(&huart1, "5. CIPSTART\r\n");

    //while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
     while(ESP8266_SendCmd(ESP8266_ONENET_INFO, ""))
        delay_ms(500);

    usart_printf(&huart1, "6. ESP8266 Init OK\r\n");

}