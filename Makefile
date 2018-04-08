



MYSQL_LIBS = `mysql_config --cflags --libs`

INCL=-I/usr/realtime/include -I/usr/src/linux/include
CFLAGS=-O2 -Wall -Wstrict-prototypes -pipe 
LIBS= -L/usr/realtime/lib -pthread -lrt $(MYSQL_LIBS)



all: trakka

trakka: trakka.c
	gcc -o trakka trakka.c $(INCL) $(CFLAGS) $(LIBS)


clean:
	-rm *~ *.o trakka
	
