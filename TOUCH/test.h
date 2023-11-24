#ifndef __TEST_H
#define __TEST_H

#include "main.h"
void Load_Drow_Dialog(void);
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color);
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color);
u16 my_abs(u16 x1,u16 x2);
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color);
u8 rtp_test(void);
void LCD_China_PWM(u16 x,u16 y);
//u8 PWM_Display(void);//œ‘ æPWM
void LCD_China_tuichu(u16 x,u16 y);
void LCD_tu(u16 x0,u16 y0);


#endif

