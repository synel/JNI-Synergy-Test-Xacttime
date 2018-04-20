GCC = arm-linux-gnueabi-gcc
OPT = -march=armv5te 
CLASS_PATH = ../bin

CFLAGS = -Wall -shared -g
IDIR = ./include
JDIRS = /usr/lib/jvm/default-java/include 
JDIRS2 = /usr/lib/jvm/default-java/include/linux
JAVAINC =$(foreach d, $(JDIRS), -I$d)
LIBPATH= ./lib
ODIR = ./target
LIBS = -lfunc

LDFLAGS=-I$(IDIR) $(JAVAINC) -L$(LIBPATH)
TARGET = libsynergy2416.so



all: synergy2416.c
	$(GCC) $(CFLAGS) $(OPT) -I$(IDIR) -I$(JDIRS) -I$(JDIRS2) synergy2416.c _fp.c _fphlp.c base64.c -o $(ODIR)/libsynergy2416.so 
	#scp libsynergy2416.so /home/chaol/ftp/.

clean:
	rm -f $(ODIR)/*.so
	rm -f *.o
