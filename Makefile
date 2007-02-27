CFLAGS  = -g -Wall -O3
LDFLAGS = -lreadline
LDPTHREAD = -lpthread

all: myshells syns synthreads java

myshells: mysh1 mysh2 mysh3 mysh4
syns: syn1 syn2
synthreads: synthread1 synthread2
java: syn1.class

synthread%: synthread%.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LDPTHREAD)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

%.class: %.java
	$(JAVAC) $<

clean:
	rm -f *.o *core *~ mysh1 mysh2 mysh3 mysh4 syn1 syn2 synthread1 synthread2
