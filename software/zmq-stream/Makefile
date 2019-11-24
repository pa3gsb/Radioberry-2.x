# Get git commit version and date
GIT_DATE := $(firstword $(shell git --no-pager show --date=short --format="%ai" --name-only))
GIT_VERSION := $(shell git describe --abbrev=0 --tags)

CC=gcc
LINK=gcc

OPTIONS=-g -Wall -Ofast -D GIT_DATE='"$(GIT_DATE)"' -D GIT_VERSION='"$(GIT_VERSION)"'

LIBS=-lrt -lzmq -lm -lpigpio -lpthread 

COMPILE=$(CC) $(OPTIONS) $(INCLUDES)

PROGRAM=zmqstream

SOURCES= \
zmqstream.c 

HEADERS= 


OBJS= \
zmqstream.o 

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

