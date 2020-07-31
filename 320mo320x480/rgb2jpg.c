#include <stdio.h>
#include "jpeglib.h"
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <string.h>
#include <stdlib.h>
#include "rgb2jpg.h"

#define DBG_PRINTF printf
/*
Allocate and initialize a JPEG compression object               		//分配和初始化一个compression结构体
Specify the destination for the compressed data (eg, a file)    		//指定压缩文件
Set parameters for compression, including image size & colorspace       //设置压缩参数，包括镜像大小，颜色 
jpeg_start_compress(...);                                               //启动压缩：jpeg_start_compress
while (scan lines remain to be written)
	jpeg_write_scanlines(...);                                          //循环调用jpeg_write_scanlines
jpeg_finish_compress(...);                                              //jpeg_finish_compress
Release the JPEG compression object                                     //释放compression结构体
*/
int rgb2jpg(struct fb_info *fb_info,unsigned char *filename)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * outfile;
	int row_stride,i;
	JSAMPROW row_pointer[1];
	// 分配和初始化一个compression结构体
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	// 指定源文件
	if ((outfile = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		return -1;
	}
	jpeg_stdio_dest(&cinfo, outfile);

    //设置压缩参数，包括镜像大小，颜色 
	cinfo.image_width = 320; 	/* image width and height, in pixels */
	cinfo.image_height = 480;
	cinfo.input_components = 4;		/* # of color components per pixel */
//	cinfo.in_color_space = JCS_EXT_RGBA; 	/* colorspace of input image */
	cinfo.in_color_space = JCS_EXT_BGRA; 	/* colorspace of input image */
    jpeg_set_defaults(&cinfo);

	// 启动解压：jpeg_start_decompress	
	jpeg_start_compress(&cinfo, TRUE);

	// 一行的数据长度
	row_stride = cinfo.image_width * cinfo.input_components;

	// 循环调用jpeg_write_scanlines
	while (cinfo.next_scanline < cinfo.image_height) 
	{
		row_pointer[0] = &((unsigned char *)(fb_info->ptr))[cinfo.next_scanline * row_stride];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	fclose(outfile);
	jpeg_destroy_compress(&cinfo);

	return 0;
}
