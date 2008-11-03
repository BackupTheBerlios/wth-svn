#
# wthd Makefile
# 
# Volker Jahns, volker@thalreit.de
#
CFLAGS=-Wall

objects = wthd.o ws2000.o pcwsr.o util.o net.o cmd.o

wthdb.o: wthdb.c
	$(CC) -I. -I/usr/local/include -I/usr/include $(CFLAGS) -c wthdb.c

$(objects): %.o: %.c
	$(CC) -I. -I/usr/local/include -I/usr/include $(CFLAGS) -c $< -o $@

wthd : $(objects) wthdb.o wth.h
	cc -L/usr/local/lib -lpthread -lsqlite3 -lrrd_th -o wthd $(objects) wthdb.o

clean:
	rm -fr wthd *.o *~
