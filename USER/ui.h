#ifndef __UI_H
#define __UI_H

#include "stm32f4xx_hal.h"
#include "lcd.h"
#include "stmflash.h"
#include "touch.h"
#include "tim.h"


extern const char *kbd_menu[];
extern const char *kbd_delFR[];

void LCD_show(void);
void LCD_Touch_Scan(void);
void LCD_AS608_UI(void);
void AS608_load_keyboard(uint16_t x, uint16_t y, uint8_t **kbtbl);
#endif

