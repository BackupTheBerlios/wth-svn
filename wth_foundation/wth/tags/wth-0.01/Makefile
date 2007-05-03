SOURCES = "wthtsts.c wthtstd.c error.c wrapunix.c wrapsock.c"
OBJECTS = 

# weatherstation stuff
wthlib.o : wthlib.c wthlib.h global.h config.h dates.h const.h net.h serial.h util.h
	gcc -c -Wall wthlib.c

serial.o : serial.c serial.h global.h config.h const.h dates.h util.h
	gcc -c -Wall serial.c

util.o :  util.c util.h global.h const.h unp.h
	gcc -c -Wall util.c

wrapunix.o : wrapunix.c unp.h
	gcc -c -Wall wrapunix.c

wrapsock.o : wrapsock.c unp.h
	gcc -c -Wall wrapsock.c

error.o : error.c unp.h
	gcc -c -Wall error.c

net.o : net.c config.h dates.h unp.h util.h
	gcc -c -Wall net.c

wthlib.a : wthlib.o wthlib.h serial.o util.o net.o error.o wrapunix.o
	ar rv wthlib.a wthlib.o serial.o util.o net.o error.o wrapunix.o
	ranlib wthlib.a


wthc.o : wthc.c
	gcc -c -Wall wthc.c

wthd.o : wthd.c
	gcc -c -Wall wthd.c

wthc: wthc.o wrapsock.o wthlib.a
	gcc -Wall -o wthc wthc.o wrapsock.o wthlib.a

wthd: wthd.o wrapunix.o wrapsock.o wthlib.a
	gcc -Wall -o wthd wthd.o wrapunix.o wrapsock.o wthlib.a




# general stuff
clean :
	@echo 'Cleaning directory'
	-rm *.o
	-rm *~
	-rm wthlib.a
	-rm wthd
	-rm wthc

