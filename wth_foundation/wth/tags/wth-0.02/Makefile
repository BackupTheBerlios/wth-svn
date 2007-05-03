# weatherstation stuff
libwth.o : libwth.c wth.h 
	cc -c -Wall libwth.c

serial.o : serial.c wth.h 
	cc -c -Wall serial.c

util.o :  util.c wth.h 
	cc -c -Wall util.c

wrapunix.o : wrapunix.c wth.h
	cc -c -Wall wrapunix.c

wrapsock.o : wrapsock.c wth.h
	cc -c -Wall wrapsock.c

error.o : error.c wth.h
	cc -c -Wall error.c

net.o : net.c wth.h
	cc -c -Wall net.c

libwth.a : libwth.o wth.h serial.o util.o net.o error.o wrapunix.o
	ar rv libwth.a libwth.o serial.o util.o net.o error.o wrapunix.o
	ranlib libwth.a


wthc.o : wthc.c wth.h
	cc -c -Wall wthc.c

wthd.o : wthd.c wth.h
	cc -c -Wall wthd.c

wthc: wthc.o wrapsock.o libwth.a
	cc -Wall -o wthc wthc.o wrapsock.o libwth.a

wthd: wthd.o wrapunix.o wrapsock.o libwth.a
	cc -Wall -o wthd wthd.o wrapunix.o wrapsock.o libwth.a


all: wthc wthd

# general stuff
clean :
	@echo 'Cleaning directory'
	-rm *.o
	-rm *~
	-rm libwth.a
	-rm wthd
	-rm wthc

