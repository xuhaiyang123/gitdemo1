#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <linux/fb.h>
#include <linux/kd.h>

#include "fbdev.h"

int fb_open(int fb_num, struct fb_info *fb_info)
{
	char str[64];
	int fd,tty;
        int ret;
	tty = open("/dev/tty1", O_RDWR);

	if(ioctl(tty, KDSETMODE, KD_GRAPHICS) == -1)
		printf("Failed to set graphics mode on tty1\n");

	sprintf(str, "/dev/fb%d", fb_num);
	fd = open(str, O_RDWR);

	ASSERT(fd >= 0);

	fb_info->fd = fd;
	IOCTL1(fd, FBIOGET_VSCREENINFO, &fb_info->var);
	IOCTL1(fd, FBIOGET_FSCREENINFO, &fb_info->fix);
	//printf("offset %dx%d fb res %dx%d virtual %dx%d, line_len %d, bpp %d\n",   fb_info->var.xoffset,fb_info->var.yoffset,fb_info->var.xres, fb_info->var.yres,fb_info->var.xres_virtual, fb_info->var.yres_virtual,fb_info->fix.line_length, fb_info->var.bits_per_pixel);

	void *ptr = mmap(0,
//			fb_info->var.yres_virtual * fb_info->fix.line_length,
 			fb_info->var.yres * fb_info->fix.line_length*2,
			PROT_WRITE | PROT_READ,
			MAP_SHARED, fd, 0);
//	ASSERT(ptr != MAP_FAILED);
        printf("ggggggggggg\n");
        if (0 > ptr)	
	{
		printf("can1111111111't mmap\n");
		return -1;
	}
        printf("zzzzzzzzzzzz\n");
	fb_info->ptr = ptr;
	memset(fb_info->ptr, 0, fb_info->var.yres * fb_info->fix.line_length*2);
	
	fb_info->bgptr = malloc(fb_info->var.yres * fb_info->fix.line_length);
	ASSERT(fb_info->bgptr != NULL);

	printf("ptr = %08x, bgptr = %08x\n", fb_info->ptr, fb_info->bgptr);
	return 0;
}

void fb_close(struct fb_info *fb_info)
{
	if (fb_info->bgptr) {
		free(fb_info->bgptr);
		fb_info->bgptr = NULL;
	}
}

void fb_set_pixel(struct fb_info *fb_info, int x, int y, uint32_t pixel)
{
	uint32_t *ptr = ((uint32_t *)fb_line_ptr(fb_info, y)) + x;
	*ptr = pixel;
}

void fb_update(struct fb_info *fb_info)
{
//	memcpy(fb_info->ptr, fb_info->bgptr, fb_info->var.yres * fb_info->fix.line_length);

	int i = 0;
	int len  = 0;
	char *p_ptr, *p_bgptr;

	p_ptr = (char *)(fb_info->ptr);
	p_bgptr = (char *)(fb_info->bgptr);

	memset(fb_info->ptr, 0, fb_info->var.yres * fb_info->fix.line_length);

	len = fb_info->var.yres * fb_info->fix.line_length;

	for(i = 0; i < len; i++)
	{
		p_ptr[i] = p_bgptr[i];
//		printf("p_ptr[%d] = %d \n", i, p_ptr[i]);
	}
}

static void fb_clear_area(struct fb_info *fb_info, int x, int y, int w, int h)
{
	int i = 0;
	int loc;
	char *fbuffer = (char *)fb_info->ptr;
	struct fb_var_screeninfo *var = &fb_info->var;
	struct fb_fix_screeninfo *fix = &fb_info->fix;

	for (i = 0; i < h; i++) {
		loc = (x + var->xoffset) * (var->bits_per_pixel / 8)
			+ (y + i + var->yoffset) * fix->line_length;
		memset(fbuffer + loc, 0, w * var->bits_per_pixel / 8);
	}
}

static void fb_put_char(struct fb_info *fb_info, int x, int y, char c,
		unsigned color)
{
	int i, j, bits, loc;
	unsigned char *p8;
	unsigned short *p16;
	unsigned int *p32;
	struct fb_var_screeninfo *var = &fb_info->var;
	struct fb_fix_screeninfo *fix = &fb_info->fix;

	for (i = 0; i < 8; i++) {
		bits = fontdata_8x8[8 * c + i];
		for (j = 0; j < 8; j++) {
			loc = (x + j + var->xoffset) * (var->bits_per_pixel / 8)
				+ (y + i + var->yoffset) * fix->line_length;
			if (loc >= 0 && loc < fix->smem_len &&
					((bits >> (7 - j)) & 1)) {
				switch (var->bits_per_pixel) {
				case 8:
					p8 =  fb_info->ptr + loc;
					*p8 = color;
				case 16:
					p16 = fb_info->ptr + loc;
					*p16 = color;
					break;
				case 24:
				case 32:
					p32 = fb_info->ptr + loc;
					*p32 = color;
					break;
				}
			}
		}
	}
}

int fb_put_string(struct fb_info *fb_info, int x, int y, char *s, int maxlen,
		int color, int clear, int clearlen)
{
	int i;
	int w = 0;

	if (clear)
		fb_clear_area(fb_info, x, y, clearlen * 8, 8);

	for (i = 0; i < strlen(s) && i < maxlen; i++) {
		fb_put_char(fb_info, (x + 8 * i), y, s[i], color);
		w += 8;
	}

	return w;
}

