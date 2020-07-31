CROSS_COMPILE = arm-linux-gnueabi-

CPU = cortex-a7
#FPU = fpv4-sp-d16

AS                = $(CROSS_COMPILE)as
LD                = $(CROSS_COMPILE)ld
CC                = $(CROSS_COMPILE)gcc
CXX               = $(CROSS_COMPILE)g++
CPP               = $(CC) -E
AR                = $(CROSS_COMPILE)ar
NM                = $(CROSS_COMPILE)nm
STRIP             = $(CROSS_COMPILE)strip
OBJCOPY           = $(CROSS_COMPILE)objcopy
OBJDUMP           = $(CROSS_COMPILE)objdump
RANLIB            = $(CROSS_COMPILE)ranlib
SIZE              = $(CROSS_COMPILE)size

TOP_DIR           = $(shell pwd)

CFLAGS_inc_path   += -I$(TOP_DIR)

# CFLAGS_inc_path   += -I$(TOP_DIR)external/jpeg-9a

# for H.264 hardware codec
CFLAGS_inc_path   += -I /home/kerson/1/workspace/buildroot/output/build/libyuv-1ebf86795cb213a37f06eb1ef3713cff080568ea/include -I /home/kerson/1/workspace/buildroot/output/build/jpeg-turbo-1.4.2 -I /home/kerson/1/workspace/out/sun8iw8p1-linaro-5.3/linux/common/buildroot/host/usr/arm-buildroot-linux-gnueabi/sysroot/usr/include/freetype2
LIBS += -L /home/kerson/1/workspace/buildroot/output/build/libyuv-1ebf86795cb213a37f06eb1ef3713cff080568ea/ -lyuv -lpthread -L /home/kerson/1/workspace/out/sun8iw8p1-linaro-5.3/linux/common/buildroot/target/usr/lib -l320Img
CFLAGS += -std=gnu99 -mthumb -mabi=aapcs-linux -mlittle-endian
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -mcpu=$(CPU) -mtune=$(CPU) 
#-mfpu=$(FPU) -mfloat-abi=hard

SRC_HAL += v4l2dev.c fbdev.c lcd.c netdbg.c usbdbg.c imutil.c timeutil.c cvtcolor.c font_8x8.c bmp.c rgb2jpg.c show.c
OBJ_HAL += $(patsubst %.c,%.o,$(SRC_HAL))

CFLAGS            += -O2 -ggdb3 -DNDEBUG
CFLAGS            += $(CFLAGS_inc_path)

EXAMPLES += fpga-camera

all: $(EXAMPLES)

.PHONY: clean

fpga-camera: demo-camera.o $(OBJ_HAL)
	$(CC) $(CFLAGS) demo-camera.o $(OBJ_HAL) -o $@ -Wl,--start-group -ljpeg $(LIBS) -lfreetype -lm -Wl,--end-group,-lpthread

clean:
	rm -f *~ *.o $(EXAMPLES)
