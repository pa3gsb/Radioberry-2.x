CC=gcc
LINK=gcc

OPTIONS=-g -Wno-deprecated-declarations -march=native -O2

LIBS=-lrt -lm -lpigpio -lpthread -lasound

COMPILE=$(CC) $(OPTIONS) $(INCLUDES)

PROGRAM=radioberry

SOURCES= \
udp.c \
audio.c \
mic.c \
local_audio_discovery.c \
radioberry.c \


HEADERS= radioberry.h \
udp.h \
audio.h \
mic.h \
local_audio_discovery.h \



OBJS= \
udp.o \
audio.o \
mic.o \
local_audio_discovery.o \
radioberry.o \

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

