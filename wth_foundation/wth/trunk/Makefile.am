CFLAGS = -ggdb

bin_PROGRAMS = wthc wthd

wthc_SOURCES = wthc.c wth.h version.h
wthc_LDADD = $(top_builddir)/libwth.a 

wthd_SOURCES = wthd.c wth.h version.h
wthd_LDADD = $(top_builddir)/libwth.a

noinst_HEADERS = wth.h wthnet.h wthprv.h

lib_LIBRARIES = libwth.a
libwth_a_SOURCES = libwth.c serial.c net.c wth.h wthnet.h wthprv.h

