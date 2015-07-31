


CC = $(CROSS_COMPILE)gcc

BCM2835_SRC_DIR = ../bcm2835-1.42.git/src

# define any compile-time flags
CFLAGS = -Wall -g


# define any directories containing header files other than /usr/include

INCLUDES = -I. -I$(BCM2835_SRC_DIR)


# define library paths in addition to /usr/lib, -Lpath
LFLAGS = 

# define any libraries to link into executable:
LIBS =

# define the C source files

BCM2835_SRC = bcm2835.c


SRCS = $(addprefix $(BCM2835_SRC_DIR)/,$(BCM2835_SRC))



OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = pi2-color-detection

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(MAIN) blink-example
	@echo  "$(MAIN) successfully compiled !"

blink-example: $(OBJS) blink-example.o
	@echo "target blink"
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(OBJS) $@.o $(LFLAGS) $(LIBS)
	@echo  "$@ successfully compiled !"


$(MAIN): $(OBJS) $(MAIN).o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(OBJS) $@.o $(LFLAGS) $(LIBS)
	@echo  "$@ successfully compiled !"


# this is a suffix replacement rule for building .o's from .c's
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
