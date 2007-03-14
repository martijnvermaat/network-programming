CFLAGS = -g -Wall -O3
LDFLAGS =

all: client serv1 serv2 serv3 talk

%: %.c assignment2.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o *core *~ client serv1 serv2 serv3 talk
