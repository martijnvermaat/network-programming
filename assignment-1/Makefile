CFLAGS = -g -Wall -O3
LDFLAGS =
LDSYNTHREAD = -lpthread
JAVAC = /usr/bin/javac

all: myshell syn synthread java

myshell: mysh1 mysh2 mysh3 mysh4
syn: syn1 syn2
synthread: synthread1 synthread2
java: syn1.class syn2.class

synthread%: synthread%.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LDSYNTHREAD)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

%.class: %.java
	$(JAVAC) $<

clean:
	rm -f *.o *core *~ mysh1 mysh2 mysh3 mysh4 syn1 syn2 synthread1 synthread2 *.class
