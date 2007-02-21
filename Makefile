all: mysh1 mysh2 mysh3 mysh4 syn1 syn2 synthread1 synthread2

%: %.c
	gcc -Wall -O3 -o $@ $<

clean:
	rm -f *.o *core *~ mysh1 mysh2 mysh3 mysh4 syn1 syn2 synthread1 synthread2
