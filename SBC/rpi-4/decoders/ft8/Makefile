CFLAGS = -O3 -g

LDFLAGS = -lm

all:	recording-ft8 upload-to-pskreporter

recording-ft8:	recording-ft8.c
	$(CC) $(CFLAGS) recording-ft8.c -o recording-ft8 $(LDFLAGS)
	
upload-to-pskreporter: upload-to-pskreporter.c
	gcc $(CFLAGS) -D_GNU_SOURCE -o $@ $^

clean:
	-rm -f *.o
	-rm -f recording-ft8 upload-to-pskreporter