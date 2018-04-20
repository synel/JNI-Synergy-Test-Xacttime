
#PLATFORM = 2410
PLATFORM = 2416

ifeq ($(PLATFORM),2410)
CC=/usr/local/arm/2.95.3/bin/arm-linux-g++
else
#CC=/usr/local/arm/Sourcery_G++_Lite/bin/arm-linux-g++
CC=/home/chaol/Development/repo/synergy2416-buildroot/output/host/usr/bin/arm-none-linux-gnueabi-g++ 
endif

CFLAGS=-Wall

LIBPATH=../
LDFLAGS=-I$(LIBPATH) -L$(LIBPATH) -lfunc

TARGET=shutdown
SRC=shutdown.cpp

all: $(TARGET)
target: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -ldl -o $@ $^ $(LDFLAGS)
#	cp -rf $(TARGET) /root/Desktop/$(TARGET)
clean:
	rm -f $(TARGET) 
