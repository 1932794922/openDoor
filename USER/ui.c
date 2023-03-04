#include "ui.h"
#include "as608.h"
#include "stdio.h"
#include "lcd.h"
#include "AS608HARD.h"
const char *kbd_menu[] =
{
    "删指纹",
    "ID:",
    "录指纹",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "DEL",
    "0",
    "Enter",
}; //按键表

const char *kbd_delFR[] = {
	"返回",
	"ID:",
	"清空指纹",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"DEL",
	"0",
	"Enter",
}; //按键表


void LCD_Touch_Scan(void)
{
    tp_dev.scan(0);

    if(tp_dev.sta & TP_PRES_DOWN)			//触摸屏被按下
    {
        if(tp_dev.x[0] < 240 && tp_dev.y[0] < 320)	//判断按下区域是否在显示屏内
        {

            if(tp_dev.x[0] > 20 && tp_dev.x[0] < 120 && tp_dev.y[0] > 30 && tp_dev.y[0] < 100)
            {
                //代码写这
                LCD_Clear(BLACK);//清屏
                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
//            while(tp_dev.x[0] > 20 && tp_dev.x[0] < 120 && tp_dev.y[0] > 30 && tp_dev.y[0] < 70) //松手检测
//            {
//                tp_dev.scan(0);
//            }
            }
        }

    }



}
void  LCD_show(void)
{

}



void LCD_AS608_UI(void)
{
   
    uint8_t str[32];
    uint8_t ensure;
   	 //模块内有效指纹个数
    ensure = PS_ValidTempleteNum(&ValidN); //读库指纹个数

    if (ensure != 0x00)
        ShowErrMessage(ensure);			 //显示确认码错误信息

    ensure = PS_ReadSysPara(&AS608Para); //读参数

    if (ensure == 0x00)
    {
        sprintf((char *)str, "库容量:%d", AS608Para.PS_max - ValidN);
        LCD_ShowString(8, 60, (uint8_t *)str, RED, BLACK);
    }
    else
        ShowErrMessage(ensure);
}


//加载按键界面（尺寸x,y为240*150）
//x,y:界面起始坐标（240*320分辨率的时候，x必须为0）
void AS608_load_keyboard(uint16_t x, uint16_t y, uint8_t **kbtbl)
{
    uint16_t i;
    POINT_COLOR = RED;
    kbd_tbl = kbtbl;
    LCD_Fill(x, y, x + 240, y + 150, BLACK);
    LCD_DrawRectangle(x, y, x + 240, y + 150, RED);
    LCD_DrawRectangle(x + 80, y, x + 160, y + 150, RED);
    LCD_DrawRectangle(x, y + 30, x + 240, y + 60, RED);
    LCD_DrawRectangle(x, y + 90, x + 240, y + 120, RED);

    for (i = 0; i < 15; i++)
    {
        if (i == 1) //按键表第2个‘:’不需要中间显示
            LCD_ShowString(x + (i % 3) * 80 + 2, y + 7 + 30 * (i / 3), (uint8_t *)kbd_tbl[i], RED, BLACK);
        else
            Show_Str_Mid(x + (i % 3) * 80, y + 7 + 30 * (i / 3), (uint8_t *)kbd_tbl[i], 16, 80);
    }
}
