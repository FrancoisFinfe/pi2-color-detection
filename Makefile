


CC = $(CROSS_COMPILE)gcc

# define any compile-time flags
CFLAGS = -Wall -g


# define any directories containing header files other than /usr/include

INCLUDES = -I.


# define library paths in addition to /usr/lib, -Lpath
LFLAGS = 

# define any libraries to link into executable:
LIBS = -lm

# define the C source files

SRCS = smbus.c testBMP085.c


OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = testBMP085

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(MAIN)
	@echo  "$(MAIN) successfully compiled !"

$(MAIN): $(OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)


# this is a suffix replacement rule for building .o's from .c's
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
