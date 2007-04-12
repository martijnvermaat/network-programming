CFLAGS = -g -Wall -O3
LDFLAGS = -lnsl
RPCGEN = /usr/bin/rpcgen
JAVAC = /usr/bin/javac

all: paperserver paperclient java hotelserver

paperserver: paperserver.o paperstorage_xdr.o paperstorage_svc.o
	$(CC) $(CFLAGS) -o paperserver paperserver.o paperstorage_svc.o paperstorage_xdr.o $(LDFLAGS)

paperclient: paperclient.o paperstorage_xdr.o paperstorage_clnt.o
	$(CC) $(CFLAGS) -o paperclient paperclient.o paperstorage_clnt.o paperstorage_xdr.o $(LDFLAGS)

paperserver.o: paperserver.c paperstorage.h assignment3.h
	$(CC) $(CFLAGS) -c paperserver.c

paperclient.o: paperclient.c paperstorage.h assignment3.h
	$(CC) $(CFLAGS) -c paperclient.c

paperstorage_clnt.o: paperstorage_clnt.c paperstorage.h
	$(CC) $(CFLAGS) -c paperstorage_clnt.c

paperstorage_svc.o: paperstorage_svc.c paperstorage.h
	$(CC) $(CFLAGS) -c paperstorage_svc.c

paperstorage_xdr.o: paperstorage_xdr.c paperstorage.h
	$(CC) $(CFLAGS) -c paperstorage_xdr.c

java:
	$(JAVAC) *.java

paperstorage.h paperstorage_clnt.c paperstorage_svc.c paperstorage_xdr.c: paperstorage.x
	$(RPCGEN) paperstorage.x
	
hotelserver: SimpleHotel.class
	rmic SimpleHotel

clean:
	rm -f *.o *core *~ paperstorage.h paperstorage_clnt.c  paperstorage_svc.c  paperstorage_xdr.c paperserver paperclient *.class
