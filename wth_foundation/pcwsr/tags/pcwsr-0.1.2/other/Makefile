#
# PC weatherstation receiver
# Makefile
#
pcwsr.o : pcwsr.c 
	cc -c -Wall pcwsr.c

pcwsr: pcwsr.o
	cc  -Wall -o pcwsr pcwsr.o


timetst.o : timetst.c 
	cc -c -Wall timetst.c

timetst: timetst.o
	cc -Wall -o timetst timetst.o


all: pcwsr timetst

# general stuff
clean :
	@echo 'Cleaning directory'
	-rm *.o
	-rm *~
	-rm pcwsr
	-rm timetst

dist:
	@echo 'Creating distfile'
	cd ..
	tar cvzf pcwsr-0.1.2.tar.gz pcwsr-0.1.2/AUTHORS
