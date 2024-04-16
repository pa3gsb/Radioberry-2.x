CC=gcc
LINK=gcc
INSTALL = install
BLDDIR = $(BUILDDIR:%=%/)

OPTIONS=-g -O3 

LIBS=-lrt -lm -lpthread 

COMPILE=$(CC) $(OPTIONS) $(INCLUDES)

PROGRAM=$(BLDDIR)radioberry

SOURCES= \
measure.c \
bias.c \
register.c \
radioberry.c 

HEADERS= \
measure.h \
bias.h \
radioberry.h \
radioberry_ioctl.h \
register.h 

OBJS= \
register.o \
bias.o \
measure.o \
radioberry.o 

all: prebuild $(PROGRAM) | $(HEADERS) $(SOURCES) 

prebuild:
	rm -f version.o

$(PROGRAM): $(OBJS:%.o=$(BLDDIR)%.o) 
	$(LINK) -o $(PROGRAM) $^ $(LIBS)

$(BLDDIR)%.o: %.c
	$(COMPILE) -c -o $@ $<

clean:
	-rm -f $(BLDDIR)*.o
	-rm -f $(PROGRAM)
	
install: $(PROGRAM)
	$(INSTALL) $(PROGRAM) $(DESTDIR)/usr/local/bin

