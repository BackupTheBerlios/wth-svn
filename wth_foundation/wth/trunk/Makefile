#
# wthd Makefile
# 
# Volker Jahns, volker@thalreit.de
#
objects = wthd.o ws2000.o pcwsr.o util.o wthdb.o net.o

$(objects): %.o: %.c
	$(CC) -I. -I/usr/local/include -I/usr/include $(CFLAGS) -c $< -o $@

wthd : $(objects)
	cc -L/usr/local/lib -lpthread -lsqlite3 -lrrd_th -o wthd $(objects)

clean:
	rm -fr *.o *~
