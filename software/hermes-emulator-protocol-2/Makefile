CC=gcc
LINK=gcc

OPTIONS=-g 

LIBS=-lrt -lm -lwiringPi -lpigpio -lpthread -lasound

COMPILE=$(CC) $(OPTIONS) $(INCLUDES)

PROGRAM=hermeslite

SOURCES= \
udp.c \
audio.c \
mic.c \
local_audio_discovery.c \
hermeslite.c \


HEADERS= hermeslite.h \
udp.h \
audio.h \
mic.h \
local_audio_discovery.h \



OBJS= \
udp.o \
audio.o \
mic.o \
local_audio_discovery.o \
hermeslite.o \

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

