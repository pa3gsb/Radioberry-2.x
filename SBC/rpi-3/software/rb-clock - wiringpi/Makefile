CC=gcc
LINK=gcc

OPTIONS=-g 

LIBS=-lrt -lm -lwiringPi -lpthread 

COMPILE=$(CC) $(OPTIONS) $(INCLUDES)

PROGRAM=rbclock

SOURCES= \
rbclock.c 

HEADERS= 


OBJS= \
rbclock.o 

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

