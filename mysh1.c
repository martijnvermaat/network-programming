
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_INPUT_LENGTH 100
#define DEBUG 1

#ifdef DEBUG
#define dprint printf
#else
#define dprint (void)
#endif


int main(int argc, char **argv) {
	pid_t pid;
    char input[MAX_INPUT_LENGTH];
    char *name;
    char *exitshell = "exit";
    int status;    

    while(fgets(input, MAX_INPUT_LENGTH, stdin) != NULL) {
        input[strlen(input)-1] = 0; // overwrite newline
        if(!strcmp(input, exitshell)) { // equal to exit
            break;
        }
        pid = fork();
        if(pid <0) { perror("Fork error"); exit(1); }
        if(pid==0) { // child process
            name = strrchr(input, '/');
            if(name == NULL) {
                name = input;
            } else {
                name = name+1; // skip slash
            }
            execlp(input, name, NULL);
            perror(input); // only reached if execlp fails
            exit(1);
        } else { // parent process
            wait(&status); // wait for child to exit
        }
    }
    
    exit(0);
}
