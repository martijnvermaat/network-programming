CFLAGS = -g -Wall -O3
AR=ar
RANLIB=ranlib
LDFLAGS = -lnsl
RPCGEN = /usr/bin/rpcgen


all: papers.cgi paperview.cgi paperload.cgi

papers.cgi: papers.o paperstorage_xdr.o paperstorage_clnt.o 
	$(CC) papers.o -o papers.cgi paperstorage_clnt.o paperstorage_xdr.o $(LDFLAGS)

papers.o: papers.c paperstorage.h
	$(CC) $(CFLAGS) -c papers.c

paperview.cgi: paperview.o paperstorage_xdr.o paperstorage_clnt.o 
	$(CC) paperview.o -o paperview.cgi paperstorage_clnt.o paperstorage_xdr.o $(LDFLAGS)

paperview.o: paperview.c paperstorage.h
	$(CC) $(CFLAGS) -c paperview.c

paperload.cgi: paperload.o paperstorage_xdr.o paperstorage_clnt.o 
	$(CC) paperload.o -o paperload.cgi paperstorage_clnt.o paperstorage_xdr.o $(LDFLAGS)

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

