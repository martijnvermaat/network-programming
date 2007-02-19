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

int count_params(char *line) {
    int params = 1;
    char *p;
    
    if(!line) { return 0; }
    
    for(p = line; *p != '\0'; p++) {
        if(*p == ' ') {
            params++;
            //while(*(p+1) == ' ') {
            //    p++;
            //}
        }
    }
    return params;
}

int exec_command(char *line) {
  char **command;
  int i = 0;
  int cparams = count_params(line);

  command = (char **) malloc(sizeof(char *) * (cparams +1));
  command[0] = strtok(line, " ");
  for(i=1; i < cparams; i++) {
    command[i] = strtok(NULL, " ");
  }
  command[cparams] = (char *) NULL;            

  if(strrchr(command[0], '/') != NULL) {
    command[0] = strrchr(command[0], '/')+1;
  }
  strcpy(line, strtok(line, " "));
  execvp(line, command);
  perror(line); // only reached if execlp fails
  free(command);
  return 1;
}

int main(int argc, char **argv) {
	pid_t pid;
    char *input;
    int input_size;
    char *exitshell = "exit";
    int status;    

    while(getline(&input, &input_size, stdin) != -1) {
        input[strlen(input)-1] = 0; // overwrite newline
        if(!strcmp(input, exitshell)) { // equal to exit
          break;
        }
        pid = fork();
        if (pid != 0) free(input);
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
