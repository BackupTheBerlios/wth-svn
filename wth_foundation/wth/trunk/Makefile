#
# wthd Makefile
# 
# Volker Jahns, volker@thalreit.de
#
# include_dirs = -I. -I/usr/local/include -I/usr/include
# CFLAGS = $(include_dirs)
# req_libs = -L/usr/local/lib -lsqlite3 -lrrd_th -lpthread
objects = wthd.o ws2000.o pcwsr.o util.o wthdb.o net.o

$(objects): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

wthd : $(objects)
	cc -o wthd $(objects)

clean:
	rm -fr *.o *~
