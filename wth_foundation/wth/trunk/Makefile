#
# Makefile wth
#
# multiple architecture build
#
# Volker Jahns, <volker@thalreit.de>, November 2008
#
VPATH=src:src/1wire/common:src/1wire/lib/general/shared:src/1wire/lib/userial/shared:src/1wire/lib/userial/link/Linux:src/1wire/lib/other/libUSB

ifndef ARCH
  ARCH := $(shell ./print_arch)
  export ARCH
endif

OBJDIR  = build_$(ARCH)
SRCS    = wthd.c ws2000.c pcwsr.c util.c net.c sqlite.c pgsql.c onewire.c ds2438.c crcutil.c ioutil.c owerr.c ownetu.c owtrnu.c ds2480ut.c linuxlnk.c owllu.c owsesu.c findtype.c atod26.c temp10.c wmr9x8.c umeter.c
OBJS    = $(patsubst %c, $(OBJDIR)/%o, $(SRCS))
WTHAPP  = $(OBJDIR)/wthd 
#$(info $(OBJS))

INSTALL = /usr/bin/install -c

DEFS = -DSQLITE_THREADSAFE=1
CDEBUG = -g
CFLAGS  = $(CDEBUG) -O0 -Wall $(DEFS) 
LDFLAGS = -g

prefix = /usr/local
bindir = $(prefix)/bin

ifneq (,$(findstring Darwin,$(ARCH)))
  #$(info This is Darwin)
  include_flags = -I./src -I/usr/local/include -I/opt/local/include -I/usr/include
  ld_flags = -L/usr/local/lib -L/opt/local/lib -lpthread -lsqlite -lpq 
endif

ifneq (,$(findstring FreeBSD,$(ARCH)))
  #$(info This is FreeBSD)
  include_flags = -I./src -I./src/1wire/common  -I./src/1wire/lib/userial/shared -I/usr/local/include -I/usr/include
  ld_flags = -L/usr/local/lib -lpthread -lsqlite3 -pg
endif

ifneq (,$(findstring Linux,$(ARCH)))
  #$(info This is Linux)
  include_flags = -I./src -I./src/1wire/common  -I./src/1wire/lib/userial/shared -I/usr/include -I/usr/local/include
  ld_flags = -DSQLITE_THREADSAFE=0 -DLINUX -L/usr/local/lib -lpthread -lsqlite3 -lpq
endif

INCLUDE = $(include_flags)
LDFLAGS = $(ld_flags)

all: $(WTHAPP)

$(OBJDIR)/wthd: $(OBJS)
	$(CC) -g $(LDFLAGS) $(OBJS) -o $@ 

$(OBJDIR)/%.o: %.c wth.h
	@mkdir -p build_$(ARCH)
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@

install: $(WTHAPP)
	$(INSTALL) $(WTHAPP) $(bindir)/$(binprefix)wthd
	-test ! -f etc/wth/wth.conf || $(INSTALL) etc/wth/wth.conf /usr/local/etc/wth/wth.conf

deinstall: $(WTHAPP)
	@killall wthd
	@rm /usr/local/bin/wthd
	@rm /usr/local/etc/wth/wth.conf
	@rmdir /usr/local/etc/wth

clean:
	@rm -fr $(WTHAPP) $(OBJDIR) *~

