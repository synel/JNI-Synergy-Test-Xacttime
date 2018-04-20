
#PLATFORM = 2410
PLATFORM = 2416

ifeq ($(PLATFORM),2410)
LINK =/usr/local/arm/2.95.3/bin/arm-linux-g++
else
LINK =/usr/local/arm/Sourcery_G++_Lite/bin/arm-linux-g++
endif

CFLAGS = -Wall -W -O2
LDFLAGS = -L../
LIBS = -ldl -lfunc

TARGET=videotest
SRC=main.cpp

all: $(TARGET)
target: $(TARGET)

$(TARGET): $(SRC)
	$(LINK) $(LDFLAGS) -o $@ $^  $(LIBS)
#	cp -rf $(TARGET) /root/Desktop/$(TARGET)
clean:
	rm -f $(TARGET) 
