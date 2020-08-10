CC         = gcc
PROGRAM    = logger
CSRC       = logger.c udp.c mkdir.c
BUILDDIR   = bin
SOURCEDIR  = src
EXTDIR     = ext

# include directories
INCLUDE    = -Isrc
VPATH      = src

CFLAGS     =  $(INCLUDE)
LDFLAGS    =

COBJ := $(notdir $(CSRC:.c=.o) )
OBJS := $(patsubst %.o, $(BUILDDIR)/%.o, $(COBJ) )

all: dir $(BUILDDIR)/$(PROGRAM)

dir:
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILDDIR)/$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(BUILDDIR)/$(PROGRAM)

# clean files
clean:
	rm $(BUILDDIR)/*.o
