CFLAGS  = -g -Wall -O3
LDFLAGS = -lreadline

all: mysh1 mysh2 mysh3 mysh4 syn1 syn2

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f *.o *core *~ mysh1 mysh2 mysh3 mysh4 syn1 syn2 synthread1 synthread2
