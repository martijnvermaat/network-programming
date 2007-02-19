#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define DEBUG 1

#ifdef DEBUG
#define dprint printf
#else
#define dprint (void)
#endif

int exec_command(char *line) {
  char *name;
  name = strrchr(line, '/');
  if(name == NULL) {
    name = line;
  } else {
    name = name+1; // skip slash
  }
  execlp(line, name, NULL);
  perror(line); // only reached if execlp fails
  return 1;
}

int main(int argc, char **argv) {
	pid_t pid;
    char *input = NULL;
    int input_size;
    char *exitshell = "exit";
    int status;    

    while(getline(&input, &input_size, stdin) != -1) {
        input[strlen(input)-1] = 0; // overwrite newline
        if(!strcmp(input, exitshell)) { // equal to exit
          break;
        }
        pid = fork();
        if (pid != 0) {
          free(input);
          input = NULL;
        }
        if(pid <0) { perror("Fork error"); exit(1); }
        if(pid==0) { // child process
          exec_command(input);
          //free(input);
          exit(1);
        } else { // parent process
          wait(&status); // wait for child to exit
        }
    }

    exit(0);
}
