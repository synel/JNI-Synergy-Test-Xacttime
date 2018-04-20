GCC = /opt/arm-2010q1/bin/arm-none-linux-gnueabi-gcc
OPT = -march=armv5te 
all: synergy2416.c
	$(GCC) $(OPT) -I./include -I/usr/lib/jvm/java-6-openjdk-i386/include/linux -I/usr/lib/jvm/java-6-openjdk-i386/include -shared synergy2416.c _fp.c _fphlp.c base64.c -o libsynergy2416.so 
	cp libsynergy2416.so /home/chaol/ftp/.

clean:
	rm -f *.so
