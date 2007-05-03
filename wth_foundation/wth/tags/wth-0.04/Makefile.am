bin_PROGRAMS = wthc wthd

wthc_SOURCES = wthc.c wth.h
wthc_LDADD = $(top_builddir)/libwth.a

wthd_SOURCES = wthd.c wth.h
wthd_LDADD = $(top_builddir)/libwth.a

noinst_HEADERS = wth.h

lib_LIBRARIES = libwth.a
libwth_a_SOURCES = libwth.c serial.c net.c util.c wth.h


