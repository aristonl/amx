# Compiles wav library

# Path: wav/Makefile

all: wav.o

.c.o:
	$(CC) $(CFLAGS) -c $*.c

wav.o: wav.c wav.h
	$(CC) $(CFLAGS) -c wav.c

clean:
	rm -rf *.o
