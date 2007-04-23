CFLAGS = -g -Wall -O3
LDFLAGS = -lnsl
RPCGEN = /usr/bin/rpcgen
JAVAC = javac
RMIC = rmic


all: paperserver paperclient hotelserver hotelclient hotelgw hotelgwclient


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

paperstorage.h paperstorage_clnt.c paperstorage_svc.c paperstorage_xdr.c: paperstorage.x
	$(RPCGEN) paperstorage.x


hotelserver: HotelServer.class

hotelclient: HotelClient.class

hotelgw: HotelGateway.class

HotelServer.class: HotelServer.java SimpleHotel_Stub.class Hotel.java SimpleHotel.java
	$(JAVAC) HotelServer.java

HotelClient.class: HotelClient.java SimpleHotel_Stub.class Hotel.java Availability.java NotAvailableException.java
	$(JAVAC) HotelClient.java

HotelGateway.class: HotelGateway.java SimpleHotel_Stub.class Hotel.java Availability.java NotAvailableException.java
	$(JAVAC) HotelGateway.java

SimpleHotel_Skel.class SimpleHotel_Stub.class: SimpleHotel.class
	$(RMIC) SimpleHotel

SimpleHotel.class: SimpleHotel.java Hotel.java Room.java RoomType.java Availability.java NotAvailableException.java NotBookedException.java
	$(JAVAC) SimpleHotel.java

hotelgwclient: hotelgwclient.c assignment3.h
	$(CC) $(CFLAGS) -o hotelgwclient hotelgwclient.c


clean:
	rm -f *.o *core *~ paperstorage.h paperstorage_clnt.c  paperstorage_svc.c  paperstorage_xdr.c paperserver paperclient *.class hotelgwclient
