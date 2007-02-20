all: mysh1 mysh2 mysh3 mysh4

%: %.c
	gcc -Wall -O3 -o $@ $<

clean:
	rm -f *.o *core *~ mysh1 mysh2 mysh3 mysh4
