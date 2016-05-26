


CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++


BCM2835_SRC_DIR = ../bcm2835-1.42.git/src
RUNNING_AVERAGE_SRC_DIR = arduLib/RunningAverage/

# define any compile-time flags
CFLAGS = -Wall -g
# define any compile-time flags
CXXFLAGS = -Wall -g -ansi

# define any directories containing header files other than /usr/include

INCLUDES = -I. -I$(BCM2835_SRC_DIR) -I$(RUNNING_AVERAGE_SRC_DIR) -Ixsd -Ilibxsd


# define library paths in addition to /usr/lib, -Lpath
LFLAGS = -lstdc++ -lxerces-c

# define any libraries to link into executable:
LIBS =

# define the C source files

BCM2835_SRC = bcm2835.c


SRCS = $(addprefix $(BCM2835_SRC_DIR)/,$(BCM2835_SRC))
SRCS += smbus.c log_utils.c watchdog.c

CXXSRCS += tcs34725.cpp xsd/settings.cxx configuration.cpp
CXXSRCS += $(addprefix $(RUNNING_AVERAGE_SRC_DIR),RunningAverage.cpp)

OBJS = $(SRCS:.c=.o)
CXXOBJS = $(CXXSRCS:.cpp=.o)

# define the executable file 
MAIN = pi2-color-detection

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(MAIN) blink-example

blink-example: $(OBJS) blink-example.o
	@echo "target blink"
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(OBJS) $@.o $(LFLAGS) $(LIBS)
	@echo  "$@ successfully compiled !"


$(MAIN): $(OBJS) $(CXXOBJS) $(MAIN).o
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(OBJS) $(CXXOBJS) $@.o $(LFLAGS) $(LIBS)
	@echo  "$@ successfully compiled !"


xsd_cxx:
	xsdcxx cxx-tree --output-dir ./xsd xsd/settings.xsd


# this is a suffix replacement rule for building .o's from .c's
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
