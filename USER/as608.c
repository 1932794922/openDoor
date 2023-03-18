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


/*串口接收中断处理在stm32f1xx_it.c里面*/
uint32_t AS608Addr = 0XFFFFFFFF;//默认

uint8_t USART3_RX_BUF[USART3_MAX_RECV_LEN];                //接收缓冲,最大USART3_MAX_RECV_LEN个字节.

uint8_t Get_Device_Code[10] = {0x01, 0x00, 0x07, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b}; //口令验证

uint8_t USART3_RX_STA = 0; //串口是否接收到数据


SysPara AS608Para;//指纹模块AS608参数
uint16_t ValidN;

//串口发送一个字节
static uint8_t MYUSART_SendData(uint8_t data) {
    if (HAL_UART_Transmit(&AS608_UART, &data, 1, 0xff) == HAL_OK)
        return 0;

    return 1;
}

//发送包头
static void SendHead(void) {
    memset(USART3_RX_BUF, 0, sizeof(USART3_RX_BUF)); //发送前清空数据，因为所有都要发送包头，所以只需要在发送包头前清空即可
    MYUSART_SendData(0xEF);
    MYUSART_SendData(0x01);
}

//发送地址
static void SendAddr(void) {
    MYUSART_SendData(AS608Addr >> 24);
    MYUSART_SendData(AS608Addr >> 16);
    MYUSART_SendData(AS608Addr >> 8);
    MYUSART_SendData(AS608Addr);
}

//发送包标识,
static void SendFlag(uint8_t flag) {
    MYUSART_SendData(flag);
}

//发送包长度
static void SendLength(int length) {
    MYUSART_SendData(length >> 8);
    MYUSART_SendData(length);
}

//发送指令码
static void Sendcmd(uint8_t cmd) {
    MYUSART_SendData(cmd);
}

//发送校验和
static void SendCheck(uint16_t check) {
    MYUSART_SendData(check >> 8);
    MYUSART_SendData(check);
}

/*****************************************
函数名：uint8_t AS608_Check(void)
参数：无
功能描述：模块是否连接检测
返回值：模块连接了返回0 否则返回1
*****************************************/
static uint8_t AS608_Check(void) {
    USART3_RX_BUF[9] = 1;

    SendHead();
    SendAddr();

    for (int i = 0; i < 10; i++) {
        MYUSART_SendData(Get_Device_Code[i]);
    }

    //HAL_UART_Receive(&AS608_UART,USART3_RX_BUF,12,100);//串口三接收12个数据
    delay_ms(200);//等待200ms

    if (USART3_RX_BUF[9] == 0)
        return 0;

    return 1;
}

/*指纹模块初始化*/
uint8_t as608_init(void) {
    //设置uart3接收中断
    HAL_UART_Receive_IT(&AS608_UART, USART3_RX_BUF, sizeof(USART3_RX_BUF)); //接收数据，且产生中断
    //使能空闲中断
    __HAL_UART_ENABLE_IT(&AS608_UART, UART_IT_IDLE); //

    return AS608_Check();
}

//判断中断接收的数组有没有应答包
//waittime为等待中断接收数据的时间（单位1ms）
//返回值：数据包首地址
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

        if (USART3_RX_STA) //接收到一次数据
        {
            USART3_RX_STA = 0;
            data = strstr((const char *) USART3_RX_BUF, (const char *) str);

            if (data)
                return (uint8_t *) data;
        }
    }

    return 0;
}

//录入图像 PS_GetImage
//功能:探测手指，探测到后录入指纹图像存于ImageBuffer。
//模块返回确认字
uint8_t PS_GetImage(void) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
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

//生成特征 PS_GenChar
//功能:将ImageBuffer中的原始图像生成指纹特征文件存于CharBuffer1或CharBuffer2
//参数:BufferID --> charBuffer1:0x01	charBuffer1:0x02
//模块返回确认字
uint8_t PS_GenChar(uint8_t BufferID) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
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

//精确比对两枚指纹特征 PS_Match
//功能:精确比对CharBuffer1 与CharBuffer2 中的特征文件
//模块返回确认字
uint8_t PS_Match(void) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
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

//搜索指纹 PS_Search
//功能:以CharBuffer1或CharBuffer2中的特征文件搜索整个或部分指纹库.若搜索到，则返回页码。
//参数:  BufferID @ref CharBuffer1	CharBuffer2
//说明:  模块返回确认字，页码（相配指纹模板）
uint8_t PS_Search(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, SearchResult *p) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
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

//合并特征（生成模板）PS_RegModel
//功能:将CharBuffer1与CharBuffer2中的特征文件合并生成 模板,结果存于CharBuffer1与CharBuffer2
//说明:  模块返回确认字
uint8_t PS_RegModel(void) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
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

//储存模板 PS_StoreChar
//功能:将 CharBuffer1 或 CharBuffer2 中的模板文件存到 PageID 号flash数据库位置。
//参数:  BufferID @ref charBuffer1:0x01	charBuffer1:0x02
//       PageID（指纹库位置号）
//说明:  模块返回确认字
uint8_t PS_StoreChar(uint8_t BufferID, uint16_t PageID) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
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

//删除模板 PS_DeletChar
//功能:  删除flash数据库中指定ID号开始的N个指纹模板
//参数:  PageID(指纹库模板号)，N删除的模板个数。
//说明:  模块返回确认字
uint8_t PS_DeletChar(uint16_t PageID, uint16_t N) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
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

//清空指纹库 PS_Empty
//功能:  删除flash数据库中所有指纹模板
//参数:  无
//说明:  模块返回确认字
uint8_t PS_Empty(void) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
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

//写系统寄存器 PS_WriteReg
//功能:  写模块寄存器
//参数:  寄存器序号RegNum:4\5\6
//说明:  模块返回确认字
uint8_t PS_WriteReg(uint8_t RegNum, uint8_t DATA) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
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
        usart_printf(&DEBUG_UART, "\r\n设置参数成功！");
    else
        usart_printf(&DEBUG_UART, "\r\n%s", EnsureMessage(ensure));

    return ensure;
}

//读系统基本参数 PS_ReadSysPara
//功能:  读取模块的基本参数（波特率，包大小等)
//参数:  无
//说明:  模块返回确认字 + 基本参数（16bytes）
uint8_t PS_ReadSysPara(SysPara *p) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
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
//        usart_printf(&DEBUG_UART, "\r\n模块最大指纹容量=%d", p->PS_max);
//        usart_printf(&DEBUG_UART, "\r\n对比等级=%d", p->PS_level);
//        usart_printf(&DEBUG_UART, "\r\n地址=%x", p->PS_addr);
//        usart_printf(&DEBUG_UART, "\r\n波特率=%d", p->PS_N * 9600);
//    }
//    else
//        usart_printf(&DEBUG_UART, "\r\n%s", EnsureMessage(ensure));

    return ensure;
}

//设置模块地址 PS_SetAddr
//功能:  设置模块地址
//参数:  PS_addr
//说明:  模块返回确认字
uint8_t PS_SetAddr(uint32_t PS_addr) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
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
    AS608Addr = PS_addr; //发送完指令，更换地址
    data = JudgeStr(2000);

    if (data)
        ensure = data[9];
    else
        ensure = 0xff;

    AS608Addr = PS_addr;

    if (ensure == 0x00)
        usart_printf(&DEBUG_UART, "\r\n设置地址成功！");
    else
        usart_printf(&DEBUG_UART, "\r\n%s", EnsureMessage(ensure));

    return ensure;
}

//功能： 模块内部为用户开辟了256bytes的FLASH空间用于存用户记事本,
//	该记事本逻辑上被分成 16 个页。
//参数:  NotePageNum(0~15),Byte32(要写入内容，32个字节)
//说明:  模块返回确认字
uint8_t PS_WriteNotepad(uint8_t NotePageNum, uint8_t *Byte32) {
    uint16_t temp;
    uint8_t ensure, i;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
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

//读记事PS_ReadNotepad
//功能：  读取FLASH用户区的128bytes数据
//参数:  NotePageNum(0~15)
//说明:  模块返回确认字+用户信息
uint8_t PS_ReadNotepad(uint8_t NotePageNum, uint8_t *Byte32) {
    uint16_t temp;
    uint8_t ensure, i;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
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

//高速搜索PS_HighSpeedSearch
//功能：以 CharBuffer1或CharBuffer2中的特征文件高速搜索整个或部分指纹库。
//		  若搜索到，则返回页码,该指令对于的确存在于指纹库中 ，且登录时质量
//		  很好的指纹，会很快给出搜索结果。
//参数:  BufferID， StartPage(起始页)，PageNum（页数）
//说明:  模块返回确认字+页码（相配指纹模板）
uint8_t PS_HighSpeedSearch(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, SearchResult *p) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
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

//读有效模板个数 PS_ValidTempleteNum
//功能：读有效模板个数
//参数: 无
//说明: 模块返回确认字+有效模板个数ValidN
uint8_t PS_ValidTempleteNum(uint16_t *ValidN) {
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
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
//        usart_printf(&DEBUG_UART, "\r\n有效指纹个数=%d", (data[10] << 8) + data[11]);
//    }
//    else
//        usart_printf(&DEBUG_UART, "\r\n%s", EnsureMessage(ensure));

    return ensure;
}

//与AS608握手 PS_HandShake
//参数: PS_Addr地址指针
//说明: 模块返新地址（正确地址）
uint8_t PS_HandShake(uint32_t *PS_Addr) {
    SendHead();
    SendAddr();
    MYUSART_SendData(0X01);
    MYUSART_SendData(0X00);
    MYUSART_SendData(0X00);
    delay_ms(200);

    if (USART3_RX_STA) //接收到数据
    {
        if (//判断是不是模块返回的应答包
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

//模块应答包确认码信息解析
//功能：解析确认码错误信息返回信息
//参数: ensure
const char *EnsureMessage(uint8_t ensure) {
    const static char *p;

    switch (ensure) {
        case 0x00:
            p = "OK";
            break;

        case 0x01:
            p = " 数据包接收错误 ";
            break;

        case 0x02:
            p = "请按压传感器";
            break;

        case 0x03:
            p = "录入指纹图像失败";
            break;

        case 0x04:
            p = "指纹太干或太淡";
            break;

        case 0x05:
            p = "指纹太湿或太糊";
            break;

        case 0x06:
            p = "指纹图像太乱";
            break;

        case 0x07:
            p = "指纹特征点太少";
            break;

        case 0x08:
            p = "指纹不匹配";
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

//显示确认码错误信息
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





//录指纹

void Add_FR(void) {
    uint8_t i = 0, ensure, processnum = 0;
    uint16_t ID_NUM = 0;
    while (1) {
        switch (processnum) {
            case 0:
                i++;
                //usart_printf(&DEBUG_UART, "请按录入指纹\r\n");
                LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                Show_Str_Mid(0, 100, (uint8_t *) "请按录入指纹", 16, 240);
                ensure = PS_GetImage();
                if (ensure == 0x00) {
                    ensure = PS_GenChar(CharBuffer1); //生成特征
                    if (ensure == 0x00) {
                        //usart_printf(&DEBUG_UART, "指纹正常\r\n");
                        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                        Show_Str_Mid(0, 100, (uint8_t *) "指纹正常", 16, 240);
                        i = 0;
                        processnum = 1; //跳到第二步
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
                // usart_printf(&DEBUG_UART, "请再按一次\r\n");
                LCD_Fill(0, 120, lcddev.width, 160, BLACK);
                Show_Str_Mid(0, 120, (uint8_t *) "请再按一次", 16, 240);
                ensure = PS_GetImage();
                delay_ms(500);
                if (ensure == 0x00) {
                    ensure = PS_GenChar(CharBuffer2); //生成特征
                    if (ensure == 0x00) {
                        // usart_printf(&DEBUG_UART, "指纹正常\r\n");
                        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                        LCD_Fill(0, 120, lcddev.width, 160, BLACK);
                        Show_Str_Mid(0, 100, (uint8_t *) "指纹正常", 16, 240);
                        i = 0;
                        processnum = 2; //跳到第三步
                    } else ShowErrMessage(ensure);
                } else {
                    if (ensure != 0x02) {
                        ShowErrMessage(ensure);

                    }
                }
                break;
            case 2:
                // usart_printf(&DEBUG_UART, "对比两次指纹\r\n");
                LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                Show_Str_Mid(0, 100, (uint8_t *) "对比两次指纹", 16, 240);
                ensure = PS_Match();
                delay_ms(500);
                if (ensure == 0x00) {
                    // usart_printf(&DEBUG_UART, "对比成功\r\n");
                    LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                    Show_Str_Mid(0, 100, (uint8_t *) "对比成功", 16, 240);
                    processnum = 3; //跳到第四步
                } else {
                    // usart_printf(&DEBUG_UART, "对比失败\r\n");
                    LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                    Show_Str_Mid(0, 100, (uint8_t *) "对比失败", 16, 240);
                    ShowErrMessage(ensure);
                    i = 0;
                    processnum = 0; //跳回第一步
                }

                delay_ms(500);
                break;

            case 3:
                //usart_printf(&DEBUG_UART, "生成指纹模板\r\n");
                LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                Show_Str_Mid(0, 100, (uint8_t *) "生成指纹模板", 16, 240);
                delay_ms(500);
                ensure = PS_RegModel();

                if (ensure == 0x00) {
                    // usart_printf(&DEBUG_UART, "生成指纹模板成功\r\n");
                    LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                    Show_Str_Mid(0, 100, (uint8_t *) "生成指纹模板成功", 16, 240);
                    processnum = 4; //跳到第五步
                } else {
                    processnum = 0;
                    ShowErrMessage(ensure);
                }

                delay_ms(1000);
                break;

            case 4:
                LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                Show_Str_Mid(0, 100, (uint8_t *) "请输入储存ID,按Enter保存", 16, 240);
                Show_Str_Mid(0, 120, (uint8_t *) "0=< ID <=299", 16, 240);
                do {
                    ID_NUM = GET_NUM();
                } while (ID_NUM >= AS608Para.PS_max);        //输入ID必须小于指纹容量的最大值
                ensure = PS_StoreChar(CharBuffer2, ID_NUM); //储存模板
                if (ensure == 0x00) {
                    uint16_t ValidN;
                    LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                    uint8_t str[32];
                    sprintf((char *) str, "录入指纹成功 ID: %d", ID_NUM);
                    Show_Str_Mid(0, 100, (uint8_t *) str, 16, 240);
                    PS_ValidTempleteNum(&ValidN); //读库指纹个数
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

        if (i >= 5) //超过5次没有按手指则退出
        {
            LCD_Fill(0, 100, lcddev.width, 160, BLACK);
            break;
        }
    }
}


//刷指纹
uint8_t ensure;

void press_FR(void) {
    SearchResult seach;

    ensure = PS_GetImage();

    if (ensure == 0x00) //获取图像成功
    {
        ensure = PS_GenChar(CharBuffer1);

        if (ensure == 0x00) //生成特征成功
        {
            ensure = PS_HighSpeedSearch(CharBuffer1, 0, 99, &seach);

            if (ensure == 0x00) //搜索成功
            {
                //LCD_Clear(BLACK);//清屏 LIGHTGREEN
                LCD_Fill(0, 100, lcddev.width, 160, BLACK);
                uint8_t str[32];
                sprintf((char *) str, "验证成功:ID:%d", seach.pageID);
                //usart_printf(&DEBUG_UART, "指纹验证成功:  ID:%d 得分:%d\r\n ", seach.pageID, seach.mathscore);
                Show_Str_Mid(0, 100, (uint8_t *) str, 16, 240);
                //发送到MQTT服务器 json字符串
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
                usart_printf(&DEBUG_UART, "验证失败\r\n");
                delay_ms(1500);
            }
        } else {
            ShowErrMessage(ensure);
        };

        //usart_printf(&DEBUG_UART, "请按指纹\r\n");
        delay_ms(1500);
        LCD_Fill(0, 100, lcddev.width, 160, BLACK);
        Show_Str_Mid(0, 100, (uint8_t *) "请按指纹", 16, 240);
    }

}

//删除单个指纹
void Del_FR(void) {
    uint8_t str[32];
    uint8_t ensure;
    uint16_t ID_NUM = 0;
    LCD_Fill(0, 80, lcddev.width, 160, BLACK);
    Show_Str_Mid(0, 120, (uint8_t *) "请输入指纹ID,按Enter发送", 16, 240);
    Show_Str_Mid(0, 140, (uint8_t *) "0=< ID <=299", 16, 240);
    delay_ms(50);
    AS608_load_keyboard(0, 170, (uint8_t **) kbd_delFR);
    ID_NUM = GET_NUM(); //获取返回的数值.

    if (ID_NUM == 0xFFFF)
        goto MENU; //返回主页面
    else if (ID_NUM == 0xFF00)
        ensure = PS_Empty(); //清空指纹库
    else
        ensure = PS_DeletChar(ID_NUM, 1); //删除单个指纹

    if (ensure == 0 && ID_NUM == 0xFF00) {
        LCD_Fill(0, 80, lcddev.width, 160, BLACK);
        sprintf((char *) str, "清空指纹库成功");
        Show_Str_Mid(0, 140, (uint8_t *) str, 16, 240);
        usart_printf(&DEBUG_UART, "删除指纹成功 \r\n");
        alarmSuccess();
    } else if (ensure == 0) {
        sprintf((char *) str, "删除指纹成功ID: %d", ID_NUM);
        LCD_Fill(0, 80, lcddev.width, 160, BLACK);
        Show_Str_Mid(0, 140, (uint8_t *) str, 16, 240);
        usart_printf(&DEBUG_UART, "删除指纹成功 \r\n");
        alarmSuccess();
    } else {
        ShowErrMessage(ensure);

    }

    delay_ms(1200);
    PS_ValidTempleteNum(&ValidN); //读库指纹个数
    LCD_ShowNum(64, 60, AS608Para.PS_max - ValidN, 3, 16, RED, BLACK);

    MENU:
    LCD_Fill(0, 80, lcddev.width, 160, BLACK);
    delay_ms(50);
    AS608_load_keyboard(0, 170, (uint8_t **) kbd_menu);

}

/*清空指纹库*/
void Del_FR_Lib(void) {
    uint8_t ensure;
    usart_printf(&DEBUG_UART, "删除指纹库开始\r\n");
    ensure = PS_Empty(); //清空指纹库

    if (ensure == 0) {
        usart_printf(&DEBUG_UART, "清空指纹库成功\r\n");
    } else
        ShowErrMessage(ensure);

    delay_ms(1500);
}

