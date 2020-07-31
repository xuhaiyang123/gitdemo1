#ifndef __LCD_H__
#define __LCD_H__

#include "fbdev.h"
#include "imutil.h"

#define LCD_PIXEL_ARGB(A,R,G,B) (((A) << 24) | ((B) << 16) | ((G) << 8) | (R))
#define LCD_PIXEL_RGB(R,G,B) (((B) << 16) | ((G) << 8) | (R) | 0xFF000000)
#define LCD_PIXEL_BGR(B,G,R) LCD_PIXEL_RGB((R),(G),(B))
#define LCD_PIXEL_GRAY(d) (((d) << 24) | ((d) << 16) | ((d) << 8) | (d))


void pthread_key(void);
int  lcd_open(void);
void lcd_close(void);
void lcd_show_image(image_t *image, rectangle_t *roi);
void lcd_show_image1(unsigned int *image, rectangle_t *roi);
int show(int x, int y,unsigned short val, int x1, int y1, unsigned short val1, int x2, int y2, unsigned short val2);

#endif
