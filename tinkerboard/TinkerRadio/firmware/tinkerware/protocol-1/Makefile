CC=gcc
LINK=gcc

OPTIONS=-g -O3 

LIBS=-lrt -lm  -lpthread 

COMPILE=$(CC) $(OPTIONS) $(INCLUDES)

PROGRAM=tinkerradio

SOURCES= \
tinkerboard_io.c \
tinkerradio.c 

HEADERS= \
tinkerboard_io.h \
tinkerboard_spi.h \
tinkerradio.h

OBJS= \
tinkerradio.o 

all: prebuild $(PROGRAM) $(HEADERS) $(SOURCES) 

prebuild:
	rm -f version.o

$(PROGRAM): $(OBJS) 
	$(LINK) -o $(PROGRAM) $(OBJS) $(LIBS)

.c.o:
	$(COMPILE) -c -o $@ $<


clean:
	-rm -f *.o
	-rm -f $(PROGRAM)

