GCC = arm-linux-gnueabi-gcc
OPT = -march=armv5te 
all: synergy2416.c
	$(GCC) $(OPT) -I./include -I/usr/lib/jvm/default-java/include/linux -I/usr/lib/jvm/default-java/include -shared synergy2416.c _fp.c _fphlp.c base64.c -o libsynergy2416.so 
	#cp libsynergy2416.so /home/chaol/ftp/.

clean:
	rm -f *.so
