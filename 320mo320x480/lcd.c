/*
 * This file is part of the Rainbow Pi project.
 * Copyright (c) 2018 YuanJun <yuanjun2006@outlook.com>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * lcd.c
 *     lcd api, based on frame buffer api
 *
 */

/*
 * Please visit our E-Shop at https://mvdevice.taobao.com/
 */
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>	
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/input.h>
#include "lcd.h"
#include "cvtcolor.h"
#include "bmp.h"
#include "rgb2jpg.h"
#define EV_SYN                  0x00
#define EV_KEY                  0x01
#define EV_REL                  0x02
#define EV_ABS                  0x03
#define EV_MSC                  0x04

#define PATH  "/mnt/extsd/1.bmp"
#define PATH1 "/mnt/extsd/1.jpg"
int flg;

static struct fb_info fbinfo;

static void *thread_fun(void *p)
{
    int fd;
    struct input_event event;	
	flg=0;
    fd = open("/dev/input/event0", O_RDONLY);
    {
	while(1){
		read(fd, &event, sizeof(event));
		if(event.type==EV_KEY){
			if(event.value&&(event.code==114)){
				flg++;
				if(flg==5)
					flg=0;
				printf("flg114=%d\n",flg);
			}
                if(event.value&&(event.code==28)){
			        //flg=1;
				printf("flg28=%d\n",flg);
			}
                if(event.value&&(event.code==102)){
			        //flg=2;
				printf("flg102=%d\n",flg);
			}
				if(event.value&&(event.code==103)){
			         // flg=3;
				      printf("flg102=%d\n",flg);
				}      
				if(event.value&&(event.code==108))
				{
					printf("flg108\n");
					bmp_encode(&fbinfo,PATH,0,0,320,480);
				}
				if(event.value&&(event.code==106)){
					printf("flg106\n");
					rgb2jpg(&fbinfo, PATH1);
				}           
			}
		}
	}
	close(fd);
	pthread_exit(NULL);
}
void pthread_key(void)
{
    int ret;
    pthread_t tid2;	
    ret = pthread_create(&tid2, NULL, thread_fun, NULL);
    if (ret < 0)
    {
	printf("pthread2_create");
    }
    pthread_detach(tid2);
}

int lcd_open(void)
{
	return fb_open(0, &fbinfo);
}

void lcd_close(void)
{
	fb_close(&fbinfo);
}

void lcd_set_pixel(int x, int y, uint32_t pixel)
{
	fb_set_pixel(&fbinfo, x, y, pixel);
}

void lcd_show_buffer(const uint8_t *buffer, size_t len)
{

}

void lcd_show_image1(unsigned int *image, rectangle_t *roi)
{
/*	int x, y,m=0;
	int sx, sy;
	int dw, dh;
	dw = roi->w;
	dh = roi->h;
	sx = roi->x;
	sy = roi->y;
	//printf("t=%d\n",t);
	//for(y=0;y<t;y++){
	//	printf("f1[%d]=%d\n",y,f1[y]);
	//}
	uint32_t *d1 = (uint32_t *)fbinfo.ptr;
	for (y = 0; y < dh; y++) {
	   for (x = 0; x < dw; x++) {
			if(x+y*dw==f1[m]&&m<360)
			{
				m++;
				continue;
			}
		    *(d1+x+y*dw)=image[x+y*dw]|0xff000000;
	   }
	}*/
	lcd_show_image2(&fbinfo,image,roi);
}

void lcd_show_image(image_t *image, rectangle_t *roi)
{
	int x, y;
	int sx, sy;
	int dw, dh;
	rectangle_t roi_tmp;
	
	if (!roi) {
		roi = &roi_tmp;
		roi->x = 0;
		roi->y = 0;
		roi->w = image->w;
		roi->h = image->h;
	}
	dw = roi->w;
	dh = roi->h;
	sx = roi->x;
	sy = roi->y;
    uint32_t *d1 = (uint32_t *)fbinfo.ptr;
	for (y = 0; y < dh; y++) {
		uint32_t *s = (uint32_t *)(image->data + (sy + y) * image->w * image->bpp + sx * image->bpp);
		for (x = 0; x < dw; x++) {	
            *d1++=*s++;
		}
	}

}
int show(int x, int y,unsigned short val, int x1, int y1, unsigned short val1, int x2, int y2, unsigned short val2)
{
      show1(&fbinfo,x,y,val,x1,y1,val1,x2,y2,val2);
      return 0;
}

