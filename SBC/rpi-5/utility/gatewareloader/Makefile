ifneq ($V,1)
Q ?= @
endif

DEBUG	= -g -O0
#DEBUG	= -O3
CC	= gcc
INCLUDE	=	-I/usr/local/include
CFLAGS	=	$(DEBUG) -Wall $(INCLUDE) -Winline -pipe

LDFLAGS	=	-L/usr/local/lib
LDLIBS  =	-lpthread -lm -lgpiod

SRC	=	loadFPGA.c loadgateware.c

OBJ	=	$(SRC:.c=.o)

BINS	=	$(SRC:.c=)

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
#cur_mkfile := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))
dir_path := $(shell dirname $(mkfile_path))

all:
	$Q cat README
	$Q echo "    $(BINS)" | fmt
	$Q echo ""

really-all:	$(BINS)

loadFPGA:	loadFPGA.o
	$Q echo [link]
	$Q $(CC) -o $@ loadFPGA.o $(LDFLAGS) $(LDLIBS)

loadgateware.o:	loadgateware.c
	$Q echo [CC] $<
	$Q $(CC) -c $(DEBUG) -Wall -I. -Winline -pipe $< -o $@

loadgateware: loadgateware.o
	$Q echo [link] [$(dir_path)]
	$Q $(CC) -o $@ loadgateware.o -lpthread -lm -l:libgpiod.so.2

toggle-pin24.o:	toggle-pin24.c
	$Q echo [CC] $<
	$Q $(CC) -c $(DEBUG) -Wall -I. -Winline -pipe $< -o $@

toggle-pin24: toggle-pin24.o
	$Q echo [link] [$(dir_path)]
	$Q $(CC) -o $@ toggle-pin24.o -lpthread -lm -l:libgpiod.so.2

loadgateware3:	loadgateware3.o
	$Q echo [link]
	$Q $(CC) -o $@ loadgateware3.o $(LDFLAGS) $(LDLIBS)

.c.o:
	$Q echo [CC] $<
	$Q $(CC) -c $(CFLAGS) $< -o $@

clean:
	$Q echo "[Clean]"
	$Q rm -f $(OBJ) *~ core tags $(BINS)

tags:	$(SRC)
	$Q echo [ctags]
	$Q ctags $(SRC)

depend:
	makedepend -Y $(SRC)
