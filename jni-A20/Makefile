PLATFORM = am335x
ifeq ($(PLATFORM),	am335x)
CCSHARED = -shared -ldl #--sysroot=/opt/ti-sdk-am335x-evm/linux-devkit/sysroots/armv7ahf-vfp-neon-3.2-oe-linux-gnueabi
TOOLCHAIN = /opt/gcc-linaro-6.4.1-2017.11-x86_64_arm-linux-gnueabihf/bin
#TOOLCHAIN = /usr/local/ti-sdk-am335x-evm/linux-devkit/sysroots/i686-arago-linux/usr/bin
#/opt/ti-sdk-am335x-evm/linux-devkit/sysroots/i686-arago-linux/usr/bin
CROSS_COMPILE = $(TOOLCHAIN)/arm-linux-gnueabihf-
#CROSS_COMPILE = /usr/bin/arm-linux-gnueabihf-
endif
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar
LD = $(CROSS_COMPILE)ld

#where the java class file is located
#for jni to generate src headers

CLASS_PATH = ../bin
SOURCE_PATH = ../src
HEADER_PATH = ./include
IDIRS = ./include
IDIRS += ./include/card
IDIRS += ./include/finger
IDIRS += ./include/serial
IDIRS += ./include/serial/spi
IDIRS += ./include/gpio


CFLAGS = -Wall -fPIC
CFLAGS_THREAD = -lpthread
CFLAGS += -D _AM335X
CFLAGS += -D _A20_UBUNTU
JDIRS = /usr/lib/jvm/default-java/include 
JDIRS2 = /usr/lib/jvm/default-java/include/linux
INCLUDE_DIR = $(foreach d, $(IDIRS), -I$d)
JAVAINC =$(foreach d, $(JDIRS), -I$d)
JAVAINC +=$(foreach d, $(JDIRS2), -I$d)
LIBPATH= ./lib
ODIR = ./target
#LIBS = -ldl
SRC = ./src

LDFLAGS=$(INCLUDE_DIR) $(JAVAINC) -L$(LIBPATH)
TARGET = libSynergyX.so

_DEPS = *.h
DEPS = $(foreach d, $(IDIRS), $(patsubst %,$(d)/%,$(_DEPS)))

#_OBJ = *SynergyX.o 
#OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))
LIBSRC = \
	base64.c \
	libserial.c \
	stringchar.c \
	usr_sem.c \
	public.c \
	inifile.c \
	file.c \
	ipconfig.c \
	synergyX.c \
	SynergyUtility.c \
	finger/_fp.c \
	finger/485_finger.c \
	finger/arm9_finger.c \
	finger/sensor_info.c \
	finger/supremainc.c \
	finger/usb_finger.c \
	finger/write_bmp_file.c \
	card/uhf.c \
	gpio/gpio.c \
	gpio/gpio_am335x.c \
	serial/serial.c \
	serial/spi/spi_am335x.c \
	serial/uart/uartlib.c \
	serial/spi/spi_a20.c 
	#finger/fp_general.c \

LIBRARY_OBJS = $(addprefix $(ODIR)/, $(addsuffix .o, $(basename $(LIBSRC))))


# Define a virtual path for .class in the bin directory
vpath %.class $(CLASS_PATH)

# $* matches the target filename without the extension
.PHONY: all
#Build shared JNI library:
all: $(ODIR)/$(TARGET)
# $@ matches the target, $< matches the first dependancy
$(ODIR)/$(TARGET) : $(LIBRARY_OBJS)
	$(CC) $(CFLAGS_THREAD) $(CCSHARED) -D _AM335X -D _A20_UBUNTU $(LDFLAGS) -o $@ $^

# $@ matches the target, $< matches the first dependancy, $^ match all the dependancies on the rhs of the :
$(ODIR)/%.o: $(SRC)/%.c
#SynergyX.o : SynergyX.c SynergyX.h
	$(CC) $(CFLAGS) $(CFLAGS_THREAD) -o $@ -c $< $(INCLUDE_DIR) $(JAVAINC)
	@echo "==="$*.c #写这一行，可以打印出被处理的文件
	#$(CC) $(CFLAGS) $(CFLAGS_THREAD) $(LDFLAGS) $< -o $@


SynergyX.h : 
	javac -h $(HEADER_PATH) -cp $(CLASS_PATH) -sourcepath $(SOURCE_PATH) $(SOURCE_PATH)/com/synel/synergy/synergyX/presentation/controller/FPU.java
	#javac -classpath $(CLASS_PATH) -sourcepath $(SOURCE_PATH) $(SOURCE_PATH)/com/synel/synergy/synergyX/presentation/controller/FPU.java
	#javah -version
	#javah -classpath $(CLASS_PATH) -sourcepath $(SOURCE_PATH) -jni com.synel.synergy.synergyX.presentation.controller.FPU
	#mv *synergyX*.h $(IDIR)
        
# 构建共享库
# Build shared library
#$(LDLIBRARY):  $(LIBRARY_OBJS)
#        $(CXX) -Istdc++ $(CFLAGS_THREAD) -shared -lrt -o $@ $^ $(COMM_LIB_OBJS) $(WLT_LIB_OBJS) $(HMAC_LIB_OBJS)


#重建内嵌隐含规则
%.o:%.cpp
	$(CXX) $(CFLAGS) -o $@ -c $< $(INCLUDE_DIR)


#重建内嵌隐含规则
%.o:%.c
	$(CC) $(OPT) $(CFLAGS) $(CFLAGS_THREAD) $(LDFLAGS) -c $< -o $@
	@echo "procissing ==="$*.c #写这一行，可以打印出被处理的文件
#       $(CC) $(CFLAGS) -o $@ -c $< -I ./include/

.PHONY: install
install:
	cp -rf $(ODIR)/$(TARGET) /srv/nfs/synergyX/$(TARGET)

.PHONY: clean
clean:

	-rm -f  $(LIBRARY_OBJS)
	-rm -f $(ODIR)/$(TARGET)
	find . -name "*.o" | xargs rm -rf
	find . -name "*.so" | xargs rm -rf
#       rm -f *.o $(SRC_O) $(SRC_FILE).so *~
#       find . -name "*~"  -exec rm {} \;
