
#PLATFORM = 2410
PLATFORM = 2416

ifeq ($(PLATFORM),2410)
CC=/usr/local/arm/2.95.3/bin/arm-linux-g++
else
#CC=/usr/local/arm/Sourcery_G++_Lite/bin/arm-linux-g++
CC = /opt/arm-2010q1/bin/arm-none-linux-gnueabi-g++
endif

CFLAGS=-Wall

LIBPATH=../
LDFLAGS=-I$(LIBPATH) -L$(LIBPATH) -lfunc

TARGET=supremafingertest 
SRC=main.cpp

all: $(TARGET)
target: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -ldl -o $@ $^ $(LDFLAGS)
	cp -rf $(TARGET) ~/ftp/$(TARGET)
clean:
	rm -f $(TARGET) 
