
#PLATFORM = 2410
PLATFORM = 2416

ifeq ($(PLATFORM),2410)
CC=/usr/local/arm/2.95.3/bin/arm-linux-g++
else
CC=/usr/local/arm/Sourcery_G++_Lite/bin/arm-linux-g++
endif
CFLAGS=-Wall -ldl

LIBPATH=../
LDFLAGS=-I$(LIBPATH) -L$(LIBPATH) -lfunc

TARGET=comcommtest 
SRC=main.cpp

all: $(TARGET)
target: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
#	cp -rf $(TARGET) /root/Desktop/$(TARGET)
clean:
	rm -f $(TARGET) 
