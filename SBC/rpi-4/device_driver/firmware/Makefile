CC=gcc
LINK=gcc

GIT_VERSION := $(shell git describe --abbrev=0 --tags)

OPTIONS=-g -O3 -D GIT_VERSION='"$(GIT_VERSION)"'

LIBS=-lrt -lm -lpthread 

COMPILE=$(CC) $(OPTIONS) $(INCLUDES)

PROGRAM=radioberry

SOURCES= \
radioberry.c 

HEADERS= \
radioberry.h \
radioberry_ioctl.h

OBJS= \
radioberry.o 

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
	
install: $(PROGRAM)
	cp $(PROGRAM) /usr/local/bin

