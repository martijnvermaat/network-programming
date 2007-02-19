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
#define PIPE_DELIMITER " | "

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
  
  if(strlen(line) < 1) {
    dprint("Oei, te klein\n");
    return 1;
  }

  dprint("line: %s\n", line);

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
  dprint("line: %s command: %s\n", line, command[0]);
  dprint("line: %s command: %s\n", line, command[0]);
  dprint("line: %s command: %s\n", line, command[0]);
  fflush(stdout);
  execvp(line, command);
  perror(line); // only reached if execlp fails
  free(command);
  return 1;
}

int main(int argc, char **argv) {
	pid_t pid;
    pid_t pid2;
    char *input = NULL;
    char *sink;
    int input_size = 0;
    char *exitshell = "exit";
    int fd[2];
    int status;    

    while(getline(&input, &input_size, stdin) != -1) {

        input[strlen(input)-1] = 0; // overwrite newline
        if(!strcmp(input, exitshell)) { // equal to exit
          break;
        }

        if ((sink = strstr(input, PIPE_DELIMITER)) != NULL) {
          sink[0] = 0;  // \0 after left argument of pipe
          sink = sink + strlen(PIPE_DELIMITER);  // skip pipe delimiter
          if (pipe(fd) < 0) {
            perror("Could not create pipe");
            continue;
          }
        }

        pid = fork();

        if(pid <0) { perror("Fork error"); exit(1); }

        if(pid==0) { // child process

          if (sink != NULL) {
            close(fd[0]);
            close(STDOUT_FILENO);
            dup2(fd[1], STDOUT_FILENO);
          }

          exec_command(input);
          exit(1);

        } else { // parent process

          if (sink != NULL) {

            pid2 = fork();

            if (pid2 == 0) { // child process

              close(fd[1]);
              close(STDIN_FILENO);
              dup2(fd[0], STDIN_FILENO);

              exec_command(sink);
              exit(1);

            } else { // parent process

              close(fd[0]);
              close(fd[1]);
              waitpid(pid2, &status, 0); // wait for child to exit
              waitpid(pid, &status, 0); // wait for child to exit

            }

          }

          if(input) {
            free(input);
            input = NULL;
          }
          input_size = 0;

        }

    }

    exit(0);
}
