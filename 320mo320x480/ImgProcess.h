// ImgProcess.h: interface for the ImgProcess class.
//
//////////////////////////////////////////////////////////////////////
//#include "StdAfx.h"

//----------------------�궨��
#define IR_ROW	240
#define IR_COL	320
#define IMAGE_H	320
#define IMAGE_W	480
#define IDISP_H	320
#define IDISP_W	480
#define FIL_N	6

//----------------------�Զ������ͱ���
typedef unsigned char unint08;
typedef unsigned short unint16;
typedef struct TPOINT
{
	int posi;
	int posj;
	unsigned short val;
}TPOINT;
typedef struct ImageROI 
{
	int x;
	int y;
	int w;
	int h;
}SRCROI;

//----------------------��ͨ���ͱ���
// extern unsigned short IP_ImageBuff0[IMAGE_H*IMAGE_W];
// extern unsigned short IP_ImageBuff1[IDISP_H*IDISP_W];
extern TPOINT IP_Tpoint[3];
extern int IP_PERLIMIT;
extern unsigned short IP_max16,IP_min16,IP_mean16;
extern SRCROI IP_SrcROI;

//----------------------��������
extern void MaxMinMeans(unint16 *img,TPOINT *max_point,TPOINT *min_point,unint16 *mean);
extern void CenterFocus(unint16 *img,int x,int y,TPOINT *cen_point);
extern void LinearScaleCalc(unint16 *img,float *link1,float *linb1,float *link2,float *linb2,int *flag);
extern void LinearConverter(unsigned short *src, unsigned char *dst);
extern void LedColorMode(unsigned char gray,int flag,float scal,int *CorRGB);
extern unsigned char Bilinear(float p,float q,unsigned char x1,unsigned char x2,unsigned char x3,unsigned char x4);
extern void ImageBilinearZoom(unsigned char *Src,SRCROI ROI,unsigned char Disp_Mode,unsigned char flag,int *RGBCell);