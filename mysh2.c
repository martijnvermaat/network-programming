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


int count_params(char *line) {
    int params = 1;
    char *p;
    
    if(!line) { return 0; }
    
    for(p = line; *p != '\0'; p++) {
        if(*p == ' ') {
            params++;
            while(*(p+1) == ' ') {
                p++;
            }
        }
    }
    return params;
}

int main(int argc, char **argv) {
	pid_t pid;
    char input[MAX_INPUT_LENGTH];
    char *exitshell = "exit";
    char **command;
    int status;    

    while(fgets(input, MAX_INPUT_LENGTH, stdin) != NULL) {
        input[strlen(input)-1] = 0; // overwrite newline
        if(!strcmp(input, exitshell)) { // equal to exit
            break;
        }
               
        pid = fork();
        if(pid <0) { perror("Fork error"); exit(1); }
        if(pid==0) { // child process
            int i = 0;
            int cparams = count_params(input);

            command = (char **) malloc(sizeof(char *) * (cparams +1));
            command[0] = strtok(input, " ");
            for(i=1; i < cparams; i++) {
                command[i] = strtok(NULL, " ");
            }
            command[cparams] = (char *) NULL;            
            
            if(strrchr(command[0], '/') != NULL) {
                command[0] = strrchr(command[0], '/')+1;
            }
            strcpy(input, strtok(input, " "));
            execvp(input, command);
            perror(input); // only reached if execlp fails
            free(command);
            exit(1);
        } else { // parent process
            wait(&status); // wait for child to exit
        }
    }
    
    exit(0);
}
