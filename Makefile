CC=gcc
CFLAGS=-O3
ARM_GCC=arm-linux-gnueabihf-gcc
default: all

espx:
	$(CC) $(CFLAGS) -o covid covid.c -lpthread

rpi:
	$(ARM_GCC) covid.c -o rpi_covid -lpthread

finish:
	scp rpi_covid root@192.168.0.1:~	

.PHONY: clean

all: espx

clean:
	rm -f covid rpi_covid
