CFLAGS = -g -Wall -O3
LDFLAGS =
LDCURSE = -lncurses

all: client serv1 serv2 serv3 talk curse

curse: curse.c assignment2.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LDCURSE)

%: %.c assignment2.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f *.o *core *~ client serv1 serv2 serv3 talk
