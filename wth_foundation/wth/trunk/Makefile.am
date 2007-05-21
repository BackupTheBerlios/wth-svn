noinst_LIBRARIES = libwth.a libwthxml.a
libwth_a_SOURCES = libwth.c serial.c net.c wthutil.c wth.h wthnet.h wthprv.h 

#libwthxml_a_SOURCES = libwth.c xml/dataxml.c serial.c net.c xml/xml.c wth.h wthnet.h wthprv.h 

bin_PROGRAMS = wthc wthd wthnewd
#bin_PROGRAMS = wthc wthd wthxc wthxmld 

wthc_SOURCES = wthc.c wth.h
#wthc_LDADD = libwth.a -L/usr/local/lib -lmpatrol
wthc_LDADD = libwth.a 

wthd_SOURCES = wthd.c wth.h 
#wthd_LDADD = $(top_builddir)/libwth.a -lmpatrol -lbfd -liberty
wthd_LDADD = libwth.a

wthnewd_SOURCES = wthnew.h wthnewd.c utilnew.c pcwsr.c ws2000.c

#wthxc_SOURCES = wthxc.c wth.h 
#wthxc_LDADD = $(top_builddir)/libwthxml.a \
#		-lmd5 -lwwwcore -lwwwxml \
#		-lwwwzip -lwwwinit -lwwwapp \
#		-lwwwhtml -lwwwtelnet -lwwwnews \
#		-lwwwhttp -lwwwmime -lwwwgopher \
#		-lwwwftp -lwwwfile -lwwwdir \
#		-lwwwcache -lwwwstream -lwwwmux \
#		-lwwwtrans -lwwwutils -lxmlrpc \
#		-lxmlrpc_client -lxmlrpc_xmlparse -lxmlrpc_xmltok


#wthxmld_SOURCES = wthxmld.c wth.h 
#wthxmld_LDADD =  $(top_builddir)/libwthxml.a -lxmlrpc -lxmlrpc_abyss \
#		-lxmlrpc_abyss_server -lxmlrpc_xmlparse \
#		-lxmlrpc_xmltok \
#		-lmd5 -lwwwcore -lwwwxml \
#		-lwwwzip -lwwwinit -lwwwapp \
#		-lwwwhtml -lwwwtelnet -lwwwnews \
#		-lwwwhttp -lwwwmime -lwwwgopher \
#		-lwwwftp -lwwwfile -lwwwdir \
#		-lwwwcache -lwwwstream -lwwwmux \
#		-lwwwtrans -lwwwutils -lxmlrpc \
#		-lxmlrpc_client -lxmlrpc_xmlparse -lxmlrpc_xmltok

noinst_HEADERS = wth.h wthnet.h wthprv.h


xmltest: wthxmld wthxc
	-killall wthxmld
	-wthxmld abyss/conf/abyss.conf
	-wthxc -c0 -h localhost -x
	-wthxc -c5 -h localhost -x
	-wthxc -c6 -i5 -h localhost -x	
	-wthxc -c5 -h localhost -x
	-wthxc -c6 -i3 -h localhost -x	
	-wthxc -c5 -h localhost -x
	-wthxc -c4 -h localhost -x
	-wthxc -c5 -h localhost -x
	-wthxc -c3 -h localhost -x
	-wthxc -c5 -h localhost -x
	-killall wthxmld

EXTRA_DIST = abyss/conf/abyss.conf wth-xmlrpc php/ postgres/ example/ Flash/
