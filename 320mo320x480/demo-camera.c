#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>
#include <libyuv.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>	
#include <sys/stat.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include "imutil.h"
#include "timeutil.h"
#include "fileutil.h"
#include "v4l2dev.h"
#include "lcd.h"
#include "cvtcolor.h"
#include "netdbg.h"
#include "usbdbg.h"
#include "ImgProcess.h"

typedef struct _tagUserBuf{
				char *buf;
				unsigned int len;
}UsrBufObj, *UsrBufHandler;

static struct sockaddr_in 	server_addr,raddr;
static unsigned int opt_camera_format = V4L2_PIX_FMT_NV12;
static unsigned int opt_camera_format1 = V4L2_PIX_FMT_SGBRG8;
static char infraredFlg = 0;
static char visibleFlg = 0;
static char tempFlg = 0;
extern int flg;

static v4l2dev_t camera,camera1;
UsrBufHandler wtbuf;
int sockfd,r;

static char c[8]={0x5A,0xD1,0x01,0x55,0xFF,0xFC,0xFD,0xFF};
static char a[8]={0x5A,0xE1,0x01,0x55,0xFF,0xFC,0xFD,0xFF};
static char b[8]={0x5A,0xF1,0x01,0x66,0xFF,0xFC,0xFD,0xFF};
static char c3[8]={0x5A,0x01,0x01,0x55,0xFF,0xFC,0xFD,0xFF};

static char d[17]={0};

#define RCVPORT		"2016"

static void *udp_deal_thrFun(void *arg)
{
		char RecvBuf[20] = {0};
		socklen_t length;
		int ret;
		int num=0;
        /*while(1){
		    sleep(1);
		    printf("r=%d\n",r);
		    r=0;
		}*/	
		sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (-1 == sockfd)
		{
			perror("sockt");
			exit(1);
		}
		bzero(&server_addr, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(atoi(RCVPORT));
		inet_aton("0.0.0.0", &server_addr.sin_addr);
		if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
			perror("bind()");
			close(sockfd);
			exit(1);
		}
		length = sizeof(raddr);
		while (1)
		{
			//printf("xuhaiyang\n");
			ret = recvfrom(sockfd, RecvBuf, 20, 0, (void *)&raddr, &length);
			if (ret < 0)
			{
				if (errno == EINTR)
					continue;
				perror("rcvfrom()");
				close(sockfd);
			}
			//printf("the massage from:[%d][%s]\n", ntohs(raddr.sin_port), inet_ntoa(raddr.sin_addr));
			if(strncmp(a,RecvBuf,8)==0)
			{
				infraredFlg = 1;
				printf("infraredFlg111 = %d\n",infraredFlg);
			}
			if(strncmp(b,RecvBuf,8)==0)			
			{
				visibleFlg = 1;
				printf("visibleFlg111 = %d\n",visibleFlg);
			}
			if(strncmp(c,RecvBuf,8)==0)			
			{
				tempFlg = 1;
				printf("tempFlg111 = %d\n",tempFlg);
			}
			if(strncmp(c3,RecvBuf,8)==0)			
			{
				infraredFlg = 0;
				tempFlg = 0;
				visibleFlg = 0;
				printf("tempFlgzzzzzzzzzz\n");
			}
		}
}
int main(int argc, const char **argv)
{
	unsigned int f[153600]={0};
    unsigned short g[76800]={0};
	unsigned int h[153600]={0};
	unsigned char Y[38400]={0};
	uint8_t g1[153617]={0};
    int ret,frame_len,frameSize,frameSize1;
	int x,y,i,count,count1,k,j=0;
	rectangle_t roi,roi1;
	uint8_t *frame_ptr,*frame_ptr1,*frame;	
    uint8_t  *argb_ptr,*argb_ptr1,*argb_ptr2;
	pthread_t tid1;
	image_t img,img1;
#define flag
	frameSize = 320*240;
    frameSize1 = 320*480*3;
    count = frameSize/4;
	count1 = frameSize1 / count;
	lcd_open();
    pthread_key();
#ifdef flag  
    v4l2_open(&camera1, 1);
	v4l2_enum_fmt(&camera1);
	v4l2_query_cap(&camera1);
	v4l2_s_input(&camera1, 0);
	v4l2_s_fmt(&camera1, 320, 240, opt_camera_format1);
	v4l2_g_fmt(&camera1);
	v4l2_reqbufs(&camera1, 4);
	v4l2_stream(&camera1, 1);
	IP_SrcROI.x=0; 
    IP_SrcROI.y=0; 
    IP_SrcROI.w=IR_COL; 
    IP_SrcROI.h=IR_ROW;
#else	
	v4l2_open(&camera, 0);
	v4l2_enum_fmt(&camera);
	v4l2_query_cap(&camera);
	v4l2_s_input(&camera, 0);
	v4l2_s_fmt(&camera, 1280, 720, opt_camera_format);
	v4l2_g_fmt(&camera);
	v4l2_reqbufs(&camera, 4);
	v4l2_stream(&camera, 1);
    argb_ptr = malloc(camera.fmt.fmt.pix.width * camera.fmt.fmt.pix.height * 4);
	argb_ptr1 = malloc(640 * 360 * 4);
	argb_ptr2 = malloc(640 * 360 * 4);
    frame = malloc(frameSize1);
#endif
	wtbuf = malloc(sizeof(UsrBufObj));
	if(wtbuf != NULL)
	{
		wtbuf->buf = malloc(frameSize);
	}
	ret = pthread_create(&tid1, NULL, udp_deal_thrFun, NULL);
	if (ret < 0)
	{
		perror("pthread1_create");
		exit(1);
	}
	while (1) {	
#ifdef flag 
		if(flg==0||flg==1||flg==2||flg==3||flg==4){
			v4l2_read_frame(&camera1, (void *)&frame_ptr1, &frame_len);
			if (frame_len <= 0) {
				continue;
			}
			i=170880;
			d[0]=frame_ptr1[i];
			for(y=1;y<7;y++){
				d[y]=frame_ptr1[i+1+y];
			}
		    /*printf("Software=%d\n",frame_ptr1[i]);	
		      printf("Frame=%d\n",frame_ptr1[i+2]<<8|frame_ptr1[i+3]); 
		      printf("FPA=%d\n",frame_ptr1[i+4]<<8|frame_ptr1[i+5]); 
			  printf("Shuttle=%d\n",frame_ptr1[i+6]<<8|frame_ptr1[i+7]);*/ 
       	 	i=171592;
			for(y=0;y<10;y++){
				d[7+y]=frame_ptr1[i+y];
			}
			/*printf("rawdata=%d\n",frame_ptr1[i]<<8|frame_ptr1[i+1]); 
			  printf("realx=%d\n",frame_ptr1[i+2]); 
			  printf("y=%d\n",frame_ptr1[i+3]); 
			  printf("realx1=%d\n",frame_ptr1[i+4]);
			  printf("y1=%d\n",frame_ptr1[i+5]); 
			  printf("rawdata1=%d\n",frame_ptr1[i+6]<<8|frame_ptr1[i+7]);
			  printf("rawdata2=%d\n",frame_ptr1[i+8]<<8|frame_ptr1[i+9]);
		    */
			for(y=0;y<240;y++){
		   		memcpy(frame_ptr1+y*320,frame_ptr1+y*712,320); 
			}			
			for(y=0;y<240;y++){
           		memcpy(frame_ptr1+76800+y*640,frame_ptr1+36+(y+242)*712,640); 
			}
			if(j==0){
				i=0;
				for(y=0;y<153600;y=y+2){
		   			g[i]=frame_ptr1[76800+y]<<8| frame_ptr1[76800+y+1];
		   			i++;
				}
			}
			memcpy(frame_ptr1+230400,d,17); 
			if(infraredFlg == 1)
			{
		    	//printf("infraredFlg = %d\n",infraredFlg);
		    	//infraredFlg = 0;
			    memset(g1,0,sizeof(g1));
				for(i=0;i<76800;i++){
                    g1[i*2+1]=frame_ptr1[i];
				}
		    	wtbuf->buf[0] = 0xE1;
            	wtbuf->buf[2] = (count & 0xff000000)>>24;
				wtbuf->buf[3] = (count & 0xff0000)>>16;
				wtbuf->buf[4] = (count & 0xff00)>>8;
				wtbuf->buf[5] = count & 0xff;
		    	wtbuf->buf[6 + count] = 0xA5;
		    	wtbuf->buf[6 + count + 1] = 0x5A;
		    	wtbuf->len = count + 8;
                for(i=0;i<7;i++){
                    wtbuf->buf[1] = i+1;
                    memcpy(&(wtbuf->buf[6]), g1+count*i, count);
		      		ret =sendto(sockfd, wtbuf->buf, wtbuf->len, 0, (void *)&raddr, sizeof(raddr));
		      		if(ret<0){
						perror("sendto()");	
						exit(1);
		      		}
		     		//usleep(50000);
		    	}	
		    	wtbuf->buf[1] = 8;
				wtbuf->buf[2] = ((count+6+17) & 0xff000000)>>24;
				wtbuf->buf[3] = ((count+6+17) & 0xff0000)>>16;
				wtbuf->buf[4] = ((count+6+17) & 0xff00)>>8;
				wtbuf->buf[5] = (count+6+17) & 0xff;
		    	memcpy(&(wtbuf->buf[6]), g1+count*7, count+17);
            	*(short *)(wtbuf->buf+6+count+17) = 0x00;
		    	*(uint32_t *)(wtbuf->buf+8+count+17) = 0x00;
		    	wtbuf->buf[12 + count+17] = 0xA5;
		    	wtbuf->buf[13 + count+17] = 0x5A;
		    	wtbuf->len = count + 14+17;
		    	ret=sendto(sockfd, wtbuf->buf, wtbuf->len, 0, (void *)&raddr, sizeof(raddr));
		    	if(ret<0){
		      		perror("sendto1111()");	
		      		exit(1);
		    	}
	    	}
			if(tempFlg == 1)
			{
				//printf("tempFlg = %d\n",tempFlg);
		    	//tempFlg = 0;
			
		    	wtbuf->buf[0] = 0xD1;
            	wtbuf->buf[2] = (count & 0xff000000)>>24;
				wtbuf->buf[3] = (count & 0xff0000)>>16;
				wtbuf->buf[4] = (count & 0xff00)>>8;
				wtbuf->buf[5] = count & 0xff;
		    	wtbuf->buf[6 + count] = 0xA5;
		    	wtbuf->buf[6 + count + 1] = 0x5A;
		    	wtbuf->len = count + 8;
            	for(i=0;i<7;i++){
              		wtbuf->buf[1] = i+1;
              		memcpy(&(wtbuf->buf[6]), frame_ptr1+76800+count*i, count);
		      		ret =sendto(sockfd, wtbuf->buf, wtbuf->len, 0, (void *)&raddr, sizeof(raddr));
		      		if(ret<0){
					    perror("sendto()");	
						exit(1);
		      		}
		      		//usleep(50000);
		   		 }	
		   		 wtbuf->buf[1] = 8;
				 wtbuf->buf[2] = ((count+6+17) & 0xff000000)>>24;
				 wtbuf->buf[3] = ((count+6+17) & 0xff0000)>>16;
				 wtbuf->buf[4] = ((count+6+17) & 0xff00)>>8;
				 wtbuf->buf[5] = (count+6+17) & 0xff;
		    	 memcpy(&(wtbuf->buf[6]), frame_ptr1+76800+count*7, count+17);
				 *(short *)(wtbuf->buf+6+count+17) = 0x00;
		    	 *(uint32_t *)(wtbuf->buf+8+count+17) = 0x00;		
		    	 wtbuf->buf[12 + count+17] = 0xA5;
		    	 wtbuf->buf[13 + count+17] = 0x5A;
		   		 wtbuf->len = count + 14+17;
		    	 ret=sendto(sockfd, wtbuf->buf, wtbuf->len, 0, (void *)&raddr, sizeof(raddr));
		    	 if(ret<0){
		      		perror("sendto1111()");	
		            exit(1);
		    	 }
		      }
		      if(flg==0)
				ImageBilinearZoom(frame_ptr1,IP_SrcROI,0,1,f);
			  if(flg==1)
			    ImageBilinearZoom(frame_ptr1,IP_SrcROI,1,1,f);
		      if(flg==2)
			    ImageBilinearZoom(frame_ptr1,IP_SrcROI,2,1,f);
			  if(flg==3)
			    ImageBilinearZoom(frame_ptr1,IP_SrcROI,3,1,f);
		      if(flg==4)
			    ImageBilinearZoom(frame_ptr1,IP_SrcROI,4,1,f);
			  for(i=0;i<480;i++){
                 for(k=0;k<320;k++){
				 	h[k+i*320]=f[i+k*480]|0xff000000;
				 }
			  }
		      roi1.x = 0;
		      roi1.y = 0;
		      roi1.w = 320;
		      roi1.h = 480;
		      lcd_show_image1(h, &roi1);
		      free(frame_ptr1);
			  if(j==0){
		      	MaxMinMeans(g,&IP_Tpoint[0],&IP_Tpoint[1],&IP_mean16);		
		      	CenterFocus(g,IR_COL/2,IR_ROW/2,&IP_Tpoint[2]);
		      	//printf("IP_Tpoint[0].posi=%d,IP_Tpoint[0].posj=%d\n",IP_Tpoint[0].posi*4/3,IP_Tpoint[0].posj*3/2);	
				//printf("IP_Tpoint[1].posi=%d,IP_Tpoint[1].posj=%d\n",IP_Tpoint[1].posi*4/3,IP_Tpoint[1].posj*3/2);	
				//printf("IP_Tpoint[2].posi=%d,IP_Tpoint[2].posj=%d\n",IP_Tpoint[2].posi,IP_Tpoint[2].posj);
		      	show(IP_Tpoint[2].posi*4/3,IP_Tpoint[2].posj*3/2,IP_Tpoint[2].val,IP_Tpoint[1].posi*4/3,IP_Tpoint[1].posj*3/2,IP_Tpoint[1].val,IP_Tpoint[0].posi*4/3,IP_Tpoint[0].posj*3/2,IP_Tpoint[0].val);
			  }
		      j++;
              if(j==15)
			  j=0;
		      //r++;
		}
#else
		if(flg==8){
        	v4l2_read_frame(&camera, (void *)&frame_ptr, &frame_len);
				if (frame_len <= 0) {
					continue;
			}	
			NV12ToARGB(frame_ptr, v4l2_width(&camera),
			   frame_ptr + v4l2_width(&camera) * v4l2_height(&camera),
			   v4l2_width(&camera),
			   argb_ptr,
			   v4l2_width(&camera) * 4,
			   v4l2_width(&camera), v4l2_height(&camera));
			for(y=0;y<360;y++){
        		for(x=0;x<640;x++){
					memcpy(argb_ptr1+(x+y*640)*4,argb_ptr+(x*2+(y*2)*1280)*4,4);
				}
			}
        	for(y=0;y<640;y++){
				for(x=0;x<360;x++){
					memcpy(argb_ptr2+(x+y*360)*4,argb_ptr1+(x*640+y)*4,4);
	        	}
			} 
       		img.w = 360;
			img.h = 640;  
			img.bpp = 4;
			img.data = argb_ptr2;
		
        	roi.x = (img.w - 320)/2;
			roi.y = (img.h - 480)/2;
			roi.w = 320;
			roi.h = 480;
			if(visibleFlg == 1)
			{	
		    	printf("visibleFlg = %d\n",visibleFlg);
		    	visibleFlg = 0;
		    	for(y=0;y<roi.h;y++){
					for(x=0;x<roi.w;x++){
						uint8_t *s = (uint8_t *)(argb_ptr1+ (roi.y + y) * img.w * img.bpp + roi.x * img.bpp+x*4);	
                		memcpy(frame+y*roi.w*3+x*3, s, 3);
					}
            	}
		    	wtbuf->buf[0] = 0xF1;
				wtbuf->buf[2] = (count & 0xff000000)>>24;
				wtbuf->buf[3] = (count & 0xff0000)>>16;
				wtbuf->buf[4] = (count & 0xff00)>>8;
				wtbuf->buf[5] = count & 0xff;		
		    	wtbuf->buf[6 + count] = 0xA5;
		    	wtbuf->buf[6 + count + 1] = 0x5A;
		    	wtbuf->len = count + 8;			
		    	for(i=0;i<count1;i++){	
		      		wtbuf->buf[1] = i+1;
		      		memcpy(&(wtbuf->buf[6]), frame+count*i, count);
		      		ret=sendto(sockfd, wtbuf->buf, wtbuf->len, 0, (void *)&raddr, sizeof(raddr));
		      		if(ret<0){
						perror("sendto1111()");	
						exit(1);
		      		}
            		usleep(50000);
		    	}
			}
			lcd_show_image(&img, &roi);
			free(frame_ptr);
		}
#endif
	}
	free(argb_ptr);
	free(argb_ptr1);
	free(frame);
	free(wtbuf->buf);
	free(wtbuf);
	v4l2_close(&camera1);
	v4l2_close(&camera);
	lcd_close();
	return 0;
}

