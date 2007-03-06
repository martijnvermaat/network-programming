CFLAGS = -g -Wall -O3
LDFLAGS =

all: client server

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o *core *~ client server
