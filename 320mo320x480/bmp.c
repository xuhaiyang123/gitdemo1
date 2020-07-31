#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bmp.h"

u8 bmp_encode(struct fb_info *fb_info,u8 *filename,u16 x,u16 y,u16 width,u16 height)
{				
	FILE* f_bmp;
	u16 bmpheadsize;			//bmp头大小	   	
 	BITMAPINFO hbmp;			//bmp头	 
	u8 res=0;
	u16 tx,ty;				   	//图像尺寸
	u32 *databuf;				//数据缓存区地址	   	
	u16 pixcnt;				   	//像素计数器
	u16 bi4width;		       	//水平像素字节数	   
	int cnt;
	if(width==0||height==0)return PIC_WINDOW_ERR;			//区域错误
 	 
	databuf=(u32*)malloc(1280);		            			//开辟至少bi4width大小的字节的内存区域 ,对240宽的屏,480个字节就够了.
	if(databuf==NULL)
		return PIC_MEM_ERR;									//内存申请失败.       
	bmpheadsize=sizeof(hbmp);								//得到bmp文件头的大小   
	memset((u8*)&hbmp,0,sizeof(hbmp));						//置零空申请到的内存.	    
	hbmp.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);			//信息头大小
	hbmp.bmiHeader.biWidth=width;	 	//bmp的宽度
	hbmp.bmiHeader.biHeight=height; 	//bmp的高度
	hbmp.bmiHeader.biPlanes=1;	 		//恒为1
	hbmp.bmiHeader.biBitCount=32;	 	//bmp为16位色bmp
//	hbmp.bmiHeader.biCompression=BI_BITFIELDS;				//每个象素的比特由指定的掩码决定。
 	hbmp.bmiHeader.biSizeImage=hbmp.bmiHeader.biHeight*hbmp.bmiHeader.biWidth*hbmp.bmiHeader.biBitCount/8;//bmp数据区大小 				   
	hbmp.bmfHeader.bfType=((u16)'M'<<8)+'B';				//BM格式标志
	hbmp.bmfHeader.bfSize=bmpheadsize+hbmp.bmiHeader.biSizeImage;	//整个bmp的大小
   	hbmp.bmfHeader.bfOffBits=bmpheadsize;					//到数据区的偏移
//	hbmp.RGB_MASK[0]=0X00F800;	 							//红色掩码
//	hbmp.RGB_MASK[1]=0X0007E0;	 							//绿色掩码
//	hbmp.RGB_MASK[2]=0X00001F;	 							//蓝色掩码
	f_bmp=fopen(filename,"w+");								//尝试打开之前的文件
 	if(f_bmp==NULL){
		perror("fopen()");
 	}
	bi4width=hbmp.bmiHeader.biWidth*4;		                //刚好为4的倍数	 
	cnt=fwrite((u8*)&hbmp,1,bmpheadsize,f_bmp);            //写入BMP首部  
	if(cnt==0){
		perror("write()");
	}
	for(ty=y+height-1;hbmp.bmiHeader.biHeight;ty--)
	{
		memcpy(databuf,(u8*)(fb_info->ptr)+ty*1280,1280);
		hbmp.bmiHeader.biHeight--;
		res=fwrite(databuf,1,1280,f_bmp);//写入数据
	}
	fclose(f_bmp);	    
	free(databuf);	 	 
	return res;
}

