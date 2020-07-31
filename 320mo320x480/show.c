#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <wchar.h>
#include <sys/ioctl.h>
#include <ft2build.h>
#include "fbdev.h"

#include FT_FREETYPE_H
#include FT_GLYPH_H

typedef struct TGlyph_ { 
	FT_UInt index; /* glyph index */ 
	FT_Vector pos; /* glyph origin on the baseline */ 
	FT_Glyph image; /* glyph image */ 
} TGlyph, *PGlyph; 


#define MAX_GLYPHS  100

static struct fb_var_screeninfo var;	/* Current var */

unsigned char *fbmem,*fbmem1;
unsigned int line_width;
unsigned int pixel_width;
FT_Library	  library;
FT_Face 	  face;
FT_GlyphSlot  slot;
static int flag1=0,flag2=0;
static unsigned int a[30]={0};
static unsigned int b[30]={0};
static unsigned int f1[3000]={0};
int t=0,z=0;

typedef unsigned char uint8_t;

/* color : 0x00RRGGBB */
void lcd_put_pixel(int x, int y, unsigned int color)
{
	unsigned char *pen_8 = fbmem+y*line_width+x*pixel_width;
	unsigned char *pen1_8 = fbmem1+y*line_width+x*pixel_width;
	unsigned short *pen_16,*pen1_16;	
	unsigned int *pen_32,*pen1_32;	

	unsigned int red, green, blue;
	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;
	pen1_16 = (unsigned short *)pen1_8;
	pen1_32 = (unsigned int *)pen1_8;

	switch (var.bits_per_pixel)
	{
		case 8:
		{
			*pen_8 = color;
			*pen1_8 = color;
			break;
		}
		case 16:
		{
			/* 565 */
			red   = (color >> 16) & 0xff;
			green = (color >> 8) & 0xff;
			blue  = (color >> 0) & 0xff;
			color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
			*pen_16 = color;
			*pen1_16 = color;
			break;
		}
		case 32:
		{
			*pen_32 = 0xff000000|(color<<16)|(color<<8)|color;
			*pen1_32 = 0xff000000|(color<<16)|(color<<8)|color;
			//*pen_32 = color|0xff000000;
			break;
		}
		default:
		{
			printf("can't surport %dbpp\n", var.bits_per_pixel);
			break;
		}
	}
}



/* Replace this function with something useful. */

void
draw_bitmap( FT_Bitmap*  bitmap,
             FT_Int      x,
             FT_Int      y)
{
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;

	//printf("x = %d, y = %d\n", x, y);

  for ( i = x, p = 0; i < x_max; i++, p++ )
  {
    for ( j = y, q = 0; j < y_max; j++, q++ )
    {
      if ( i < 0      || j < 0       ||
           i >= var.xres || j >= var.yres )
        continue;
      //image[j][i] |= bitmap->buffer[q * bitmap->width + p];
      if(bitmap->buffer[q * bitmap->width + p]!=0){
	  	   f1[663+z]=j*320+i;
		   z++;
      	   lcd_put_pixel(i, j, bitmap->buffer[q * bitmap->width + p]);
      }
    }
  }
}


int Get_Glyphs_Frm_Wstr(FT_Face face, wchar_t * wstr, TGlyph glyphs[])
{
	int n;
	PGlyph glyph = glyphs;
	int pen_x = 0;
	int pen_y = 0;
	int error;
	FT_GlyphSlot  slot = face->glyph;;
	
		
	for (n = 0; n < wcslen(wstr); n++)
	{
		glyph->index = FT_Get_Char_Index( face, wstr[n]); 
		/* store current pen position */ 
		glyph->pos.x = pen_x; 
		glyph->pos.y = pen_y;		

		/* load时是把glyph放入插槽face->glyph */
		error = FT_Load_Glyph(face, glyph->index, FT_LOAD_DEFAULT);
		if ( error ) 
			continue;

		error = FT_Get_Glyph(face->glyph, &glyph->image ); 
		if ( error ) 
			continue;

		/* translate the glyph image now */ 
		/* 这使得glyph->image里含有位置信息 */
		FT_Glyph_Transform(glyph->image, 0, &glyph->pos );

		pen_x += slot->advance.x;  /* 1/64 point */

		/* increment number of glyphs */ 
		glyph++;		
	}

	/* count number of glyphs loaded */ 
	return (glyph - glyphs);
}

void compute_string_bbox(TGlyph glyphs[], FT_UInt num_glyphs, FT_BBox *abbox )
{
	FT_BBox bbox; 
	int n;
	
	bbox.xMin = bbox.yMin = 32000; 
	bbox.xMax = bbox.yMax = -32000;

	for ( n = 0; n < num_glyphs; n++ )
	{
		FT_BBox glyph_bbox;
		
		FT_Glyph_Get_CBox(glyphs[n].image, FT_GLYPH_BBOX_TRUNCATE, &glyph_bbox );

		if (glyph_bbox.xMin < bbox.xMin)
			bbox.xMin = glyph_bbox.xMin;

		if (glyph_bbox.yMin < bbox.yMin)
			bbox.yMin = glyph_bbox.yMin;

		if (glyph_bbox.xMax > bbox.xMax)
			bbox.xMax = glyph_bbox.xMax;

		if (glyph_bbox.yMax > bbox.yMax)
			bbox.yMax = glyph_bbox.yMax;
	}

	*abbox = bbox;
}


void Draw_Glyphs(TGlyph glyphs[], FT_UInt num_glyphs, FT_Vector pen)
{
	int n;
	int error;
	
	for (n = 0; n < num_glyphs; n++)
	{
		FT_Glyph_Transform(glyphs[n].image, 0, &pen);
		/* convert glyph image to bitmap (destroy the glyph copy!) */ 
		error = FT_Glyph_To_Bitmap(&glyphs[n].image, FT_RENDER_MODE_NORMAL, 0, /* no additional translation */ 
                              		1 ); 		/* destroy copy in "image" */
		if ( !error ) 
		{ 
			FT_BitmapGlyph bit = (FT_BitmapGlyph)glyphs[n].image; 
			draw_bitmap(&bit->bitmap, bit->left, var.yres - bit->top); 
			FT_Done_Glyph(glyphs[n].image ); 
		}
	}
}

int show1(struct fb_info *fb_info,int x, int y,unsigned short val, int x1, int y1, unsigned short val1, int x2, int y2, unsigned short val2)
{
	wchar_t wstr1[10],wstr2[10],wstr3[10];
	int error;
    FT_Vector     pen;	
	int i,g,k,j=0;
	FT_BBox bbox;
	int line_box_ymin = 10000;
	int line_box_ymax = 0;
	int line_box_width;
	int line_box_height;
	TGlyph glyphs[MAX_GLYPHS]; /* glyphs table */ 
	FT_UInt num_glyphs;

    swprintf(wstr1,10,L"%.1f",(-4.98429748314413e-10)*val*val*val+(1.71524598656579e-05)*val*val+(-0.167078621266143)*val+523.760638185230);
	swprintf(wstr2,10,L"%.1f",(-4.98429748314413e-10)*val1*val1*val1+(1.71524598656579e-05)*val1*val1+(-0.167078621266143)*val1+523.760638185230);
	swprintf(wstr3,10,L"%.1f",(-4.98429748314413e-10)*val2*val2*val2+(1.71524598656579e-05)*val2*val2+(-0.167078621266143)*val2+523.760638185230);
	if(flag1==0){
		flag1=1;
		var=fb_info->var;
		fbmem = fb_info->ptr;
		fbmem1 = fb_info->ptr+614400;
		line_width  = var.xres * var.bits_per_pixel / 8;
		pixel_width = var.bits_per_pixel / 8;
		/* 显示矢量字体 */
		error = FT_Init_FreeType( &library );			   /* initialize library */
		/* error handling omitted */
		error = FT_New_Face( library, "/ahellya.ttf", 0, &face ); /* create face object */
		/* error handling omitted */	
		slot = face->glyph;
		FT_Set_Pixel_Sizes(face, 24, 0);
		for(i=0;i<30;i++){
			a[i]=0xffffffff;
			b[i]=0xff00ff00;
		}
	}
    //中心点 (160,240)
	for(i=0;i<2;i++){
		memcpy(fbmem+(x-14+320*(y-14+i))*4,a,120);
		memcpy(fbmem1+(x-14+320*(y-14+i))*4,a,120);
		for(g=0;g<30;g++){
			f1[g+i*60]=x-14+320*(y-14+i)+g;
		}
		memcpy(fbmem+(x-14+320*(y+1+14-i))*4,a,120);
		memcpy(fbmem1+(x-14+320*(y+1+14-i))*4,a,120);
		for(g=0;g<30;g++){
			f1[30+g+i*60]=x-14+320*(y+1+14-i)+g;
		}
	}
    for(i=0;i<30;i++){
		memcpy(fbmem+(x-14+320*(y-14+i))*4,a,8);
		memcpy(fbmem1+(x-14+320*(y-14+i))*4,a,8);
		for(g=0;g<2;g++){
           f1[120+g+4*i]=x-14+320*(y-14+i)+g;
	    }
		memcpy(fbmem+(x+14+320*(y-14+i))*4,a,8);	
		memcpy(fbmem1+(x+14+320*(y-14+i))*4,a,8);
		for(g=0;g<2;g++){
           f1[122+g+4*i]=x+14+320*(y-14+i)+g;
	    }
	}
	for(i=0;i<15;i++){
		memcpy(fbmem+(x+320*(y-14-i))*4,a,4);
		memcpy(fbmem1+(x+320*(y-14-i))*4,a,4);
	    f1[240+i*8]=x+320*(y-14-i);
		memcpy(fbmem+(x+1+320*(y-14-i))*4,a,4);	
		memcpy(fbmem1+(x+1+320*(y-14-i))*4,a,4);	
	    f1[241+i*8]=x+1+320*(y-14-i);
		memcpy(fbmem+(x+320*(y+1+14+i))*4,a,4);
		memcpy(fbmem1+(x+320*(y+1+14+i))*4,a,4);
		f1[242+i*8]=x+320*(y+1+14+i);
		memcpy(fbmem+(x+1+320*(y+1+14+i))*4,a,4);
		memcpy(fbmem1+(x+1+320*(y+1+14+i))*4,a,4);
		f1[243+i*8]=x+1+320*(y+1+14+i);
		memcpy(fbmem+(x-14-i+320*y)*4,a,4);
		memcpy(fbmem1+(x-14-i+320*y)*4,a,4);
		f1[244+i*8]=x-14-i+320*y;
		memcpy(fbmem+(x-14-i+320*(y+1))*4,a,4);
		memcpy(fbmem1+(x-14-i+320*(y+1))*4,a,4);
		f1[245+i*8]=x-14-i+320*(y+1);
		memcpy(fbmem+(x+1+14+i+320*y)*4,a,4);
		memcpy(fbmem1+(x+1+14+i+320*y)*4,a,4);
		f1[246+i*8]=x+1+14+i+320*y;
		memcpy(fbmem+(x+1+14+i+320*(y+1))*4,a,4);
		memcpy(fbmem1+(x+1+14+i+320*(y+1))*4,a,4);
		f1[247+i*8]=x+1+14+i+320*(y+1);
	}
	//printf("11111111111111\n");
	//x横坐标，y纵坐标，最低温度(100,200)
	//x1=40,y1=420;
	if(x1<40)
		x1=40;
	if(y1<20)
		y1=20;
	if(x1>280)
		x1=280;
	if(y1>420)
		y1=420;
	memcpy(fbmem+(x1-5+y1*320)*4,b,44);
	memcpy(fbmem1+(x1-5+y1*320)*4,b,44);
	for(g=0;g<11;g++){
		f1[360+g]=x1-5+y1*320+g;
	}
	for(i=0;i<11;i++){		
		memcpy(fbmem+(x1+(y1-5+i)*320)*4,b,4);
		memcpy(fbmem1+(x1+(y1-5+i)*320)*4,b,4);
		f1[371+i]=x1+(y1-5+i)*320;
	}
	for(i=0;i<2;i++){
		memcpy(fbmem+(x1-13+(y1-13+i)*320)*4,b,32);
		memcpy(fbmem1+(x1-13+(y1-13+i)*320)*4,b,32);
		for(g=0;g<8;g++){
			f1[382+g+i*32]=x1-13+(y1-13+i)*320+g;
		}
		memcpy(fbmem+(x1+13-7+(y1-13+i)*320)*4,b,32);
		memcpy(fbmem1+(x1+13-7+(y1-13+i)*320)*4,b,32);
		for(g=0;g<8;g++){
			f1[390+g+i*32]=x1+13-7+(y1-13+i)*320+g;
		}
		memcpy(fbmem+(x1-13+(y1+13-i)*320)*4,b,32);
		memcpy(fbmem1+(x1-13+(y1+13-i)*320)*4,b,32);
		for(g=0;g<8;g++){
			f1[398+g+i*32]=x1-13+(y1+13-i)*320+g;
		}
		memcpy(fbmem+(x1+13-7+(y1+13-i)*320)*4,b,32);
		memcpy(fbmem1+(x1+13-7+(y1+13-i)*320)*4,b,32);
		for(g=0;g<8;g++){
			f1[406+g+i*32]=x1+13-7+(y1+13-i)*320+g;
		}
	}
	for(i=0;i<8;i++){
		memcpy(fbmem+(x1-13+(y1-13+i)*320)*4,b,4);
		memcpy(fbmem1+(x1-13+(y1-13+i)*320)*4,b,4);
	    f1[446+i*8]=x1-13+(y1-13+i)*320;
		memcpy(fbmem+(x1-13+(y1+13-i)*320)*4,b,4);
		memcpy(fbmem1+(x1-13+(y1+13-i)*320)*4,b,4);
	    f1[447+i*8]=x1-13+(y1+13-i)*320;
		memcpy(fbmem+(x1+13+(y1-13+i)*320)*4,b,4);
		memcpy(fbmem1+(x1+13+(y1-13+i)*320)*4,b,4);
		f1[448+i*8]=x1+13+(y1-13+i)*320;
	    memcpy(fbmem+(x1+13+(y1+13-i)*320)*4,b,4);
		memcpy(fbmem1+(x1+13+(y1+13-i)*320)*4,b,4);
		f1[449+i*8]=x1+13+(y1+13-i)*320;
		memcpy(fbmem+(x1-13+1+(y1-13+i)*320)*4,b,4);
		memcpy(fbmem1+(x1-13+1+(y1-13+i)*320)*4,b,4);
		f1[450+i*8]=x1-13+1+(y1-13+i)*320;
		memcpy(fbmem+(x1-13+1+(y1+13-i)*320)*4,b,4);
		memcpy(fbmem1+(x1-13+1+(y1+13-i)*320)*4,b,4);
		f1[451+i*8]=x1-13+1+(y1+13-i)*320;
		memcpy(fbmem+(x1+13-1+(y1-13+i)*320)*4,b,4);
		memcpy(fbmem1+(x1+13-1+(y1-13+i)*320)*4,b,4);
		f1[452+i*8]=x1+13-1+(y1-13+i)*320;
	    memcpy(fbmem+(x1+13-1+(y1+13-i)*320)*4,b,4);
		memcpy(fbmem1+(x1+13-1+(y1+13-i)*320)*4,b,4);
		f1[453+i*8]=x1+13-1+(y1+13-i)*320;
    }
	//printf("22222222222\n");	
    //x横坐标，y纵坐标，最高温度(300,300)
    //x2=280;y2=20;
   	if(x2<40)
		x2=40;
	if(y2<20)
		y2=20;
	if(x2>280)
		x2=280;
	if(y2>420)
		y2=420;  
	memcpy(fbmem+(x2-2+(y2-2)*320)*4,a,20);
	memcpy(fbmem1+(x2-2+(y2-2)*320)*4,a,20);
	for(g=0;g<5;g++){
     	f1[510+g]= x2-2+(y2-2)*320+g;
	}
	memcpy(fbmem+(x2-2+(y2-1)*320)*4,a,20);
	memcpy(fbmem1+(x2-2+(y2-1)*320)*4,a,20);
	for(g=0;g<5;g++){
     	f1[515+g]= x2-2+(y2-1)*320+g;
	}
	memcpy(fbmem+(x2-2+y2*320)*4,a,20);
	memcpy(fbmem1+(x2-2+y2*320)*4,a,20);
	for(g=0;g<5;g++){
     	f1[520+g]= x2-2+y2*320+g;
	}
	memcpy(fbmem+(x2-2+(y2+1)*320)*4,a,20);
	memcpy(fbmem1+(x2-2+(y2+1)*320)*4,a,20);
	for(g=0;g<5;g++){
     	f1[525+g]= x2-2+(y2+1)*320+g;
	}
	memcpy(fbmem+(x2-2+(y2+2)*320)*4,a,20);
	memcpy(fbmem1+(x2-2+(y2+2)*320)*4,a,20);
	for(g=0;g<5;g++){
     	f1[530+g]= x2-2+(y2+2)*320+g;
	}
	for(i=0;i<2;i++){
		memcpy(fbmem+(x2-13+(y2-13+i)*320)*4,a,32);
		memcpy(fbmem1+(x2-13+(y2-13+i)*320)*4,a,32);
		for(g=0;g<8;g++){
			f1[535+g+i*32]=x2-13+(y2-13+i)*320+g;
		}
		memcpy(fbmem+(x2+13-7+(y2-13+i)*320)*4,a,32);
		memcpy(fbmem1+(x2+13-7+(y2-13+i)*320)*4,a,32);
	    for(g=0;g<8;g++){
			f1[543+g+i*32]=x2+13-7+(y2-13+i)*320+g;
		}
		memcpy(fbmem+(x2-13+(y2+13-i)*320)*4,a,32);
		memcpy(fbmem1+(x2-13+(y2+13-i)*320)*4,a,32);
		for(g=0;g<8;g++){
			f1[551+g+i*32]=x2-13+(y2+13-i)*320+g;
		}
		memcpy(fbmem+(x2+13-7+(y2+13-i)*320)*4,a,32);
		memcpy(fbmem1+(x2+13-7+(y2+13-i)*320)*4,a,32);
		for(g=0;g<8;g++){
			f1[559+g+i*32]=x2+13-7+(y2+13-i)*320+g;
		}	
	}
	for(i=0;i<8;i++){
		memcpy(fbmem+(x2-13+(y2-13+i)*320)*4,a,4);
		memcpy(fbmem1+(x2-13+(y2-13+i)*320)*4,a,4);
		f1[599+i*8]=x2-13+(y2-13+i)*320;
		memcpy(fbmem+(x2-13+(y2+13-i)*320)*4,a,4);
		memcpy(fbmem1+(x2-13+(y2+13-i)*320)*4,a,4);
		f1[600+i*8]=x2-13+(y2+13-i)*320;
		memcpy(fbmem+(x2+13+(y2-13+i)*320)*4,a,4);
		memcpy(fbmem1+(x2+13+(y2-13+i)*320)*4,a,4);
		f1[601+i*8]=x2+13+(y2-13+i)*320;
	    memcpy(fbmem+(x2+13+(y2+13-i)*320)*4,a,4);
		memcpy(fbmem1+(x2+13+(y2+13-i)*320)*4,a,4);
		f1[602+i*8]=x2+13+(y2+13-i)*320;
		memcpy(fbmem+(x2-13+1+(y2-13+i)*320)*4,a,4);
		memcpy(fbmem1+(x2-13+1+(y2-13+i)*320)*4,a,4);
		f1[603+i*8]=x2-13+1+(y2-13+i)*320;
		memcpy(fbmem+(x2-13+1+(y2+13-i)*320)*4,a,4);
		memcpy(fbmem1+(x2-13+1+(y2+13-i)*320)*4,a,4);
		f1[604+i*8]=x2-13+1+(y2+13-i)*320;
		memcpy(fbmem+(x2+13-1+(y2-13+i)*320)*4,a,4);
		memcpy(fbmem1+(x2+13-1+(y2-13+i)*320)*4,a,4);
		f1[605+i*8]=x2+13-1+(y2-13+i)*320;
	    memcpy(fbmem+(x2+13-1+(y2+13-i)*320)*4,a,4);
		memcpy(fbmem1+(x2+13-1+(y2+13-i)*320)*4,a,4);
		f1[606+i*8]=x2+13-1+(y2+13-i)*320;
    }
	// wstr1
	num_glyphs = Get_Glyphs_Frm_Wstr(face, wstr1, glyphs);
	compute_string_bbox(glyphs, num_glyphs, &bbox);
	line_box_width  = bbox.xMax - bbox.xMin;
	line_box_height = bbox.yMax - bbox.yMin;
    //printf("line_box_width1=%d,line_box_height1=%d\n",line_box_width,line_box_height);
	pen.x = (x-line_box_width/2) * 64;
	pen.y = (var.yres-(y+62-line_box_height/2))*64;
	Draw_Glyphs(glyphs, num_glyphs, pen);	
	// wstr2
	num_glyphs = Get_Glyphs_Frm_Wstr(face, wstr2, glyphs);
	compute_string_bbox(glyphs, num_glyphs, &bbox);
	line_box_width  = bbox.xMax - bbox.xMin;
	line_box_height = bbox.yMax - bbox.yMin;
    //printf("line_box_width1=%d,line_box_height1=%d\n",line_box_width,line_box_height);
	pen.x = (x1-line_box_width/2) * 64;
	pen.y = (var.yres-(y1+50-line_box_height/2))*64;
	Draw_Glyphs(glyphs, num_glyphs, pen);
	//wstr3
    num_glyphs = Get_Glyphs_Frm_Wstr(face, wstr3, glyphs);
	compute_string_bbox(glyphs, num_glyphs, &bbox);
	line_box_width  = bbox.xMax - bbox.xMin;
	line_box_height = bbox.yMax - bbox.yMin;
	//printf("line_box_width2=%d,line_box_height2=%d\n",line_box_width,line_box_height);
	pen.x = (x2-line_box_width/2) * 64;
	pen.y = (var.yres-(y2+50-line_box_height/2))*64;
	Draw_Glyphs(glyphs, num_glyphs, pen);
	t=663+z-1;
	for(i=0;i<t;i++)
    {
		for(g=i+1;g<t;g++){
			if(f1[i]>f1[g]){
				k=f1[i];
				f1[i]=f1[g];
                f1[g]=k;
			}
		}
	}
	for(i=0;i<t;i++){
		if(f1[i]==f1[i+1])
		{
			continue;
		}
        f1[j]=f1[i];
		j++;

	}
	z=0;
	return 0;	
}
void lcd_show_image2(struct fb_info *fb_info,unsigned int *image, rectangle_t *roi)
{
	int x, y,ret,m=0;
	int sx, sy;
	int dw, dh;
	dw = roi->w;
	dh = roi->h;
	sx = roi->x;
	sy = roi->y;
	uint32_t *d1=(uint32_t *)fb_info->ptr;;
        if(flag2==0){
		for (y = 0; y < dh; y++) {
	   		for (x = 0; x < dw; x++) {
				if(x+y*dw==f1[m]&&m<t)
				{
					m++;
					continue;
				}
		    		*(d1+x+y*dw)=image[x+y*dw];
	   		}
		}
        	fb_info->var.yoffset=0;
        	IOCTL1(fb_info->fd, FBIOPAN_DISPLAY, &fb_info->var);
                flag2=1;
	}else{
                d1 = d1+153600; 
		for (y = 0; y < dh; y++) {
	   		for (x = 0; x < dw; x++) {
				if(x+y*dw==f1[m]&&m<t)
				{
					m++;
					continue;
				}
		    		*(d1+x+y*dw)=image[x+y*dw];
	   		}

		}
        	fb_info->var.yoffset=480;
        	IOCTL1(fb_info->fd, FBIOPAN_DISPLAY, &fb_info->var);
                flag2=0;
	}
	//memset(fb_info->ptr, 0, fb_info->var.yres * fb_info->fix.line_length);
        /*if(flag2==0){
          memcpy(d1,image,320*480*4);
          fb_info->var.yoffset=0;
          IOCTL1(fb_info->fd, FBIOPAN_DISPLAY, &fb_info->var);
	  flag2=1;
        }else{
          memcpy(d1+153600,image,320*480*4);
          fb_info->var.yoffset=480;
          IOCTL1(fb_info->fd, FBIOPAN_DISPLAY, &fb_info->var);
	  flag2=0;
        }*/	          
}

