all: mysh1 mysh2

mysh1: mysh1.c
	gcc -Wall -O3 -o mysh1 mysh1.c

mysh2: mysh2.c
	gcc -Wall -O3 -o mysh2 mysh2.c

clean:
	rm -f *.o *core *~ mysh1 mysh2 mysh3
