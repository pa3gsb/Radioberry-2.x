CC=gcc
LINK=gcc

OPTIONS=-g -O3 

LIBS=-lrt -lm -lpigpio -lpthread 

COMPILE=$(CC) $(OPTIONS) $(INCLUDES)

PROGRAM=performance

SOURCES= \
performance.c 

HEADERS= \
performance.h

OBJS= \
performance.o 

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

