#include "ui.h"
#include "as608.h"
#include "stdio.h"
#include "lcd.h"
#include "AS608HARD.h"
#include "funHandle.h"

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

uint8_t rC522Flag = 3;

void LCD_RC522_Scan(void) {
    tp_dev.scan(0);
    if (tp_dev.sta & TP_PRES_DOWN)            //触摸屏被按下
    {
        if (tp_dev.x[0] < 240 && tp_dev.y[0] < 320)    //判断按下区域是否在显示屏内
        {
            if (tp_dev.x[0] > 180 && tp_dev.x[0] < 240 && tp_dev.y[0] > 20 && tp_dev.y[0] < 60) {
                //点击了录卡
                LCD_Clear(BLACK);//清屏
                LCD_ShowString(54, 0, (uint8_t *) "----正在录卡----", RED, BLACK);
                LCD_DrawRectangle(100, 20, 160, 60, RED);
                LCD_ShowString(115, 32, (uint8_t *) "HOME", RED, BLACK);
                rC522Flag = 1; //执行录卡
                while (tp_dev.x[0] > 180 && tp_dev.x[0] < 240 && tp_dev.y[0] > 20 && tp_dev.y[0] < 60) //松手检测
                {
                    tp_dev.scan(0);
                }
            }

            if (tp_dev.x[0] > 0 && tp_dev.x[0] < 60 && tp_dev.y[0] > 20 && tp_dev.y[0] < 60) {
                //点击了删卡
                LCD_Clear(BLACK);//清屏
                LCD_ShowString(54, 0, (uint8_t *) "----正在删卡----", RED, BLACK);
                LCD_DrawRectangle(100, 20, 160, 60, RED);
                LCD_ShowString(115, 32, (uint8_t *) "HOME", RED, BLACK);
                rC522Flag = 2; //执行录卡
                while (tp_dev.x[0] > 0 && tp_dev.x[0] < 60 && tp_dev.y[0] > 20 && tp_dev.y[0] < 60) //松手检测
                {
                    tp_dev.scan(0);
                }
            }

            if (tp_dev.x[0] > 100 && tp_dev.x[0] < 160 && tp_dev.y[0] > 20 && tp_dev.y[0] < 60) {
                //点击了Home
                LCD_Clear(BLACK);//清屏
                AS608_load_keyboard(0, 170, (uint8_t **) kbd_menu); //加载虚拟键盘
                LCD_RC522_UI();
                LCD_AS608_UI();
                rC522Flag = 3; //执行录卡
                while (tp_dev.x[0] > 20 && tp_dev.x[0] < 220 && tp_dev.y[0] > 20 && tp_dev.y[0] < 60) //松手检测
                {
                    tp_dev.scan(0);
                }
            }
        }

    }


}

/**
 * RFID模块UI
 */
void LCD_RC522_UI(void) {
    LCD_DrawRectangle(180, 20, lcddev.width, 50, RED);
    LCD_DrawRectangle(0, 20, 60, 50, RED);
    LCD_ShowString(200, 28, (uint8_t *) "录卡", RED, BLACK);
    LCD_ShowString(10, 28, (uint8_t *) "删卡", RED, BLACK);
}

/**
 * 指纹模块UI
 */
void LCD_AS608_UI(void) {
    uint8_t str[32];
    uint8_t ensure;
    uint16_t ValidN;
    //模块内有效指纹个数
    LCD_ShowString(54, 0, (uint8_t *) "----门禁系统----", RED, BLACK);
    ensure = PS_ValidTempleteNum(&ValidN); //读库指纹个数
    if (ensure != 0x00) {
        //ShowErrMessage(ensure);             //显示确认码错误信息
    }
    ensure = PS_ReadSysPara(&AS608Para); //读参数
    if (ensure == 0x00) {
        sprintf((char *) str, "库容量:%d", AS608Para.PS_max - ValidN);
        LCD_ShowString(8, 60, (uint8_t *) str, RED, BLACK);
    } else {
        //ShowErrMessage(ensure);
    }
}


//加载按键界面（尺寸x,y为240*150）
//x,y:界面起始坐标（240*320分辨率的时候，x必须为0）
void AS608_load_keyboard(uint16_t x, uint16_t y, uint8_t **kbtbl) {
    uint16_t i;
    POINT_COLOR = RED;
    kbd_tbl = kbtbl;
    LCD_Fill(x, y, x + 240, y + 150, BLACK);
    LCD_DrawRectangle(x, y, x + 240, y + 150, RED);
    LCD_DrawRectangle(x + 80, y, x + 160, y + 150, RED);
    LCD_DrawRectangle(x, y + 30, x + 240, y + 60, RED);
    LCD_DrawRectangle(x, y + 90, x + 240, y + 120, RED);
    for (i = 0; i < 15; i++) {
        if (i == 1) //按键表第2个‘:’不需要中间显示
            LCD_ShowString(x + (i % 3) * 80 + 2, y + 7 + 30 * (i / 3), (uint8_t *) kbd_tbl[i], RED, BLACK);
        else
            Show_Str_Mid(x + (i % 3) * 80, y + 7 + 30 * (i / 3), (uint8_t *) kbd_tbl[i], 16, 80);
    }
}
