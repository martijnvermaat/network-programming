CFLAGS = -g -Wall -O3
AR=ar
RANLIB=ranlib
LIBS=-L./ -lcgic -lnsl
RPCGEN = /usr/bin/rpcgen


all: libcgic.a papers.cgi paperview.cgi paperload.cgi

libcgic.a: cgic.o cgic.h
	rm -f libcgic.a
	$(AR) rc libcgic.a cgic.o
	$(RANLIB) libcgic.a

papers.cgi: papers.o libcgic.a paperstorage_xdr.o paperstorage_clnt.o 
	$(CC) papers.o -o papers.cgi paperstorage_clnt.o paperstorage_xdr.o ${LIBS}

papers.o: papers.c paperstorage.h
	$(CC) $(CFLAGS) -c papers.c

paperview.cgi: paperview.o libcgic.a paperstorage_xdr.o paperstorage_clnt.o 
	$(CC) paperview.o -o paperview.cgi paperstorage_clnt.o paperstorage_xdr.o ${LIBS}

paperview.o: paperview.c paperstorage.h
	$(CC) $(CFLAGS) -c paperview.c

paperload.cgi: paperload.o libcgic.a paperstorage_xdr.o paperstorage_clnt.o 
	$(CC) paperload.o -o paperload.cgi paperstorage_clnt.o paperstorage_xdr.o ${LIBS}

paperload.o: paperload.c paperstorage.h
	$(CC) $(CFLAGS) -c paperload.c

paperstorage_clnt.o: paperstorage_clnt.c paperstorage.h
	$(CC) $(CFLAGS) -c paperstorage_clnt.c

paperstorage_xdr.o: paperstorage_xdr.c paperstorage.h
	$(CC) $(CFLAGS) -c paperstorage_xdr.c

paperstorage.h paperstorage_clnt.c paperstorage_xdr.c: paperstorage.x
	$(RPCGEN) paperstorage.x

clean:
	rm -f *.o *.a *core *~ papers.cgi paperview.cgi paperload.cgi paperstorage.h paperstorage_clnt.c paperstorage_svc.c paperstorage_xdr.c

