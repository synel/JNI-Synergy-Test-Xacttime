
#PLATFORM = x86
#PLATFORM = 2410
PLATFORM = 2416

ifeq ($(PLATFORM),2410)
CC=/usr/local/arm/2.95.3/bin/arm-linux-gcc
else
#CC=/usr/local/arm/Sourcery_G++_Lite/bin/arm-linux-gcc
CC=/home/chaol/Development/repo/synergy2416-buildroot/output/host/usr/bin/arm-none-linux-gnueabi-gcc
endif


CFLAGS=-Wall 
LIBPATH=.
AR = ar


ifeq ($(PLATFORM),2410)
LIBSRC=	./crypt/crypt.c 	\
		./gpio/gpio.c	\
		./camera/camera.c	\
	./setdate/set_date.c \
	./public/public.c	\
	./public/protocol.c \
	./public/initcom.c	\
	./card/readcard.c	\
	./weigand/weigand.c \
	./printer/s310.c	\
	./printer/printer.c	\
	./finger/finger.c	\
	./sound/sound.h	\
	./gprs/gprs.c	\
	./net/net_tcp.c	\
	./com/com.c	\
	./serial/serial.c	\
	./serial/uart/uartlib.c	\
	./key/key.c		
else
LIBSRC=	./crypt/crypt.c 	\
		./gpio/gpio.c	\
		./camera/camera.c	\
	./camera/dynctrl.c	\
	./camera/spcav4l2.c	\
	./camera/utils.c	\
	./camera/v4l2uvc.c	\
	./setdate/set_date.c \
	./public/public.c	\
	./public/protocol.c \
	./public/initcom.c	\
	./card/readcard.c	\
	./weigand/weigand.c \
	./printer/s310.c	\
	./printer/printer.c	\
	./finger/finger.c	\
	./finger/smackbio.c	\
	./finger/supremainc.c	\
	./sound/sound.h	\
	./gprs/gprs.c	\
	./gprs/em310.c	\
	./gprs/m35.c	\
	./net/net_tcp.c	\
	./com/com.c	\
	./serial/serial.c	\
	./serial/uart/uartlib.c	\
	./serial/spi/spilib.c	\
	./serial/spi/spi_s3c2416.c	\
	./key/key.c	\
	./power/power.c		
endif		


LIB=$(LIBPATH)/libfunc.a
LIB2=$(LIBPATH)/libfunc.so
LIBOBJ=$(addprefix $(LIBPATH)/, $(addsuffix .o, $(basename $(LIBSRC))))

all: $(LIB) $(LIB2) 
lib: $(LIB)

$(LIBPATH)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(LIB): $(LIBOBJ)
	$(AR) -rcs $@ $^
	
$(LIB2): $(LIBOBJ)
	$(CC) -fPIC -shared -o $@ $^	
	
ifeq ($(PLATFORM),2410)	
	$(AR) -x libcav41.a    
	$(AR) -r libfunc.a spcav4l.o		 	
endif

	cp -rf libfunc.a ../demo/libfunc.a	
#   cp -rf libfunc.so ../demo/libfunc.so	
#   cp -rf libfunc.a /qt-4.5.3/s3c2416/lib/libfunc.a	
clean:
	rm -f $(LIB) $(LIB2) $(LIBOBJ)
