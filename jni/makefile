#PLATFORM = i386
#PLATFORM = 2410
PLATFORM = 2416

ifeq ($(PLATFORM),	i386)
CC = /usr/bin/g++
OPT = -march=i386
endif

ifeq ($(PLATFORM),2410)
CC = /usr/local/arm/2.95.3/bin/arm-linux-g++
OPT = -march=armv4e
else
CC = /opt/arm-2010q1/bin/arm-none-linux-gnueabi-g++
OPT = -march=armv5te 
endif

#where the java class file is located
#for jni to generate src headers
CLASS_PATH = ../bin

CFLAGS = -Wall, -shared
IDIR = ./include
JDIRS = /usr/lib/jvm/java-7-openjdk-amd64/include /usr/lib/jvm/java-7-openjdk-amd64/include/linux
JAVAINC =$(foreach d, $(JDIRS), -I$d)
LIBPATH= ./lib
ODIR = ./target
LIBS = -lfunc

LDFLAGS=-I$(IDIR) $(JAVAINC) -L$(LIBPATH)
TARGET = libsynergy2416jni.so

_DEPS = *Synergy2416JNI.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = Synergy2416JNI.o SynergyUtility.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

# Define a virtual path for .class in the bin directory
vpath %.class $(CLASS_PATH)

# $* matches the target filename without the extension
.PHONY: all
all: $(ODIR)/$(TARGET)
# $@ matches the target, $< matches the first dependancy
$(ODIR)/$(TARGET) : $(OBJ)
	$(CC) $(OPT) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
	cp -rf $(ODIR)/$(TARGET) ~/ftp/$(TARGET)

# $@ matches the target, $< matches the first dependancy, $^ match all the dependancies on the rhs of the :
$(ODIR)/%.o: %.c $(DEPS)
#Synergy2416JNI.o : Synergy2416JNI.c Synergy2416JNI.h
	$(CC) $(OPT) $(LDFLAGS) -c  $< -o $@  


Synergy2416JNI.h : 
	javah -classpath $(CLASS_PATH) -jni com.synel.synergy.synergy2416.jni.Synergy2416JNI
	mv *Synergy2416JNI*.h $(IDIR)
	
.PHONY: clean	
clean:
	rm -f $(ODIR)/*.so $(ODIR)/*.o *~ $(IDIR)/*Synergy2416JNI*.h