#
# Makefile wth
#
# multiple architecture build
#
# Volker Jahns, <volker@thalreit.de>, November 2008
#
VPATH=src

ifndef ARCH
  ARCH := $(shell ./print_arch)
  export ARCH
endif

OBJDIR  = build_$(ARCH)
SRCS    = wthd.c ws2000.c pcwsr.c util.c net.c cmd.c wthdb.c
OBJS    = $(patsubst %c, $(OBJDIR)/%o, $(SRCS))
WTHAPP  = $(OBJDIR)/wthd
$(info $(OBJS))

CFLAGS  = -Wall
CC = gcc

ifneq (,$(findstring Darwin,$(ARCH)))
  $(info This is Darwin)
  include_flags = -I./src -I/usr/local/include -I/opt/local/include -I/usr/include
  ld_flags = -L/usr/local/lib -L/opt/local/lib -lpthread -lrrd_th 
endif

ifneq (,$(findstring FreeBSD,$(ARCH)))
  $(info This is FreeBSD)
  include_flags = -I./src -I/usr/local/include -I/usr/include
  ld_flags = -L/usr/local/lib -lpthread -lrrd_th -lsqlite3
endif

INCLUDE = $(include_flags)
LDFLAGS = $(ld_flags)

all: $(WTHAPP)

$(WTHAPP): $(OBJS)
	$(CC) -g $(LDFLAGS) $(OBJS) -o $@ 

$(OBJDIR)/%.o: %.c wth.h
	@mkdir -p build_$(ARCH)
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@

clean:
	@rm -fr $(WTHAPP) $(OBJDIR) *~

