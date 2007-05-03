# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/sh

srcdir = .
top_srcdir = ../../..
prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/xmlrpc-c
pkglibdir = $(libdir)/xmlrpc-c
pkgincludedir = $(includedir)/xmlrpc-c

top_builddir = ../../..

ACLOCAL = aclocal
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/ginstall -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
host_alias = i686-pc-linux-gnu
host_triplet = i686-pc-linux-gnu
ABYSS_SUBDIR = abyss
AS = @AS@
ASYNCH_CLIENT = asynch_client
AUTH_CLIENT = auth_client
AVAILABLE_MODULES = c++ cgi-server abyss-server libwww-client 
CC = gcc
CLIENTTEST = clienttest
CONFIGURE_DATE = Wed Dec  5 15:12:15 CET 2001
CPPTEST = cpptest
CXX = c++
DLLTOOL = @DLLTOOL@
EFRPCTEST = 
EFRPCTEST_WRAPPER = 
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LIBWWW_CFLAGS = -I/usr/include/w3c-libwww -DHAVE_CONFIG_H
LIBWWW_CONFIG = /usr/bin/libwww-config
LIBWWW_LDADD = -L/usr/lib -lwwwxml -lxmltok -lxmlparse -lwwwzip -lwwwinit -lwwwapp -lwwwhtml -lwwwtelnet -lwwwnews -lwwwhttp -lwwwmime -lwwwgopher -lwwwftp -lwwwfile -lwwwdir -lwwwcache -lwwwstream -lwwwmux -lwwwtrans -lwwwcore -lwwwutils -lmd5 -ldl -lz
LIBWWW_LIBDIR = /usr/lib
LIBWWW_RPATH = -rpath /usr/lib
LIBWWW_WL_RPATH = -Wl,--rpath -Wl,/usr/lib
LIBXMLRPC_ABYSS_SERVER_LA = libxmlrpc_abyss_server.la
LIBXMLRPC_CGI_LA = libxmlrpc_cgi.la
LIBXMLRPC_CLIENT_LA = libxmlrpc_client.la
LIBXMLRPC_CPP_A = libxmlrpc_cpp.a
LN_S = ln -s
MAKEINFO = makeinfo
MEERKAT_APP_LIST = meerkat-app-list
OBJDUMP = @OBJDUMP@
PACKAGE = xmlrpc-c
QUERY_MEERKAT = query-meerkat
RANLIB = ranlib
SAMPLE_CGI_CGI = sample-cgi.cgi
SERVER = server
SERVERTEST = servertest
SYNCH_CLIENT = synch_client
VALIDATEE = validatee
VERSION = 0.9.9
VERSION_INFO = -version-info 6:0:3
XMLRPCCPP_H = XmlRpcCpp.h
XMLRPC_ABYSS_H = xmlrpc_abyss.h
XMLRPC_CGI_H = xmlrpc_cgi.h
XMLRPC_CLIENT_H = xmlrpc_client.h
XML_RPC_API2CPP_SUBDIR = xml-rpc-api2cpp

EXTRA_DIST = index.htm pwrabyss.gif
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../../../xmlrpc_config.h
CONFIG_CLEAN_FILES = 
DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = gtar
GZIP_ENV = --best
all: all-redirect
.SUFFIXES:
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu --include-deps lib/abyss/htdocs/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status

tags: TAGS
TAGS:


distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = lib/abyss/htdocs

distdir: $(DISTFILES)
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$d/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am:
install-exec: install-exec-am

install-data-am:
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am:
uninstall: uninstall-am
all-am: Makefile
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:


mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-generic clean-am
	-rm -f libtool

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: tags distdir info-am info dvi-am dvi check check-am \
installcheck-am installcheck install-exec-am install-exec \
install-data-am install-data install-am install uninstall-am uninstall \
all-redirect all-am all installdirs mostlyclean-generic \
distclean-generic clean-generic maintainer-clean-generic clean \
mostlyclean distclean maintainer-clean


# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
