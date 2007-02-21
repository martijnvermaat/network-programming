#define DEBUG 1

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>


#ifdef DEBUG
#define dprint printf
#else
#define dprint (void)
#endif


int _malloc_counter = 0;

void *counting_malloc(size_t siz) {
    _malloc_counter++;
    return malloc(siz);
}

void counting_free(void *mem) {
    _malloc_counter--;
    free(mem);
}

#define malloc counting_malloc
#define free counting_free


#define PROMPT_STRING "%s> "
#define COMMAND_EXIT "exit"
#define COMMAND_CD "cd"
#define COMMAND_PIPE " | "


/*
  Count number of tokens seperated by one or more delimiter occurrences.
*/
int num_tokens (char *line, char delimiter) {

    int count = 0;
    char *position = line;

    if (!line) {
        return 0;
    }

    // Find tokens while not at end of line
    while (*position != '\0') {

        // Skip delimiters
        while (*position == delimiter) {
            position++;
        }

        // Found token
        if (*position != '\0') {

            count++;

            // Skip to end of token
            while (*position != delimiter && *position != '\0') {
                position++;
            }

        }

    }

    return count;

}


/*
  Get tokens seperated by one or more delimiter occurrences.
  Original line will be modified!
  Return number of tokens read.
*/
int get_tokens (char *line, char delimiter, char **tokens, size_t tokens_length) {

    int count = 0;
    char *position = line;

    if (!line) {
        return 0;
    }

    // Find tokens while not at end of line
    while (*position != '\0') {

        // Check for overflow
        if (count >= tokens_length) {
            break;
        }

        // Skip delimiters
        while (*position == delimiter) {
            position++;
        }

        // Found token
        if (*position != '\0') {

            // Store token
            tokens[count] = position;
            count++;

            // Skip to end of token
            while (*position != delimiter && *position != '\0') {
                position++;
            }

            // End of token
            if (*position != '\0') {
                *position = '\0';
                position++;
            }

        }

    }

    return count;

}


/*
  Execute the command with possible parameters seperated by spaces.
*/
int exec_command (char *line) {

    char **arguments;
    char *command;
    int num_arguments;

    if (!line) {
        return -1;
    }

    num_arguments = num_tokens(line, ' ');

    if (num_arguments < 1) {
        return -1;
    }

    // Allocate array of pointers to arguments
    arguments = (char **) malloc(sizeof(char *) * (num_arguments + 1));

    get_tokens(line, ' ', arguments, num_arguments);

    // End argument list with NULL pointer
    arguments[num_arguments] = (char *) NULL;

    // Store executable command
    command = arguments[0];

    // Skip preceding path for executable name
    if(strrchr(arguments[0], '/') != NULL) {
        arguments[0] = strrchr(arguments[0], '/') + 1;
    }

    execvp(command, arguments);

    // Only reached on execvp error
    perror(command);
    free(arguments);

    return -1;

}


int main(int argc, char **argv) {

    pid_t pid;
    pid_t pid2;
    char *input = NULL;
    char *sink;
    size_t input_size = 0;
    int fd[2];
    int status;    

    // TODO: it seems we sometimes need an fflush before the readline

    while ((input = readline(PROMPT_STRING)) != NULL) {  // TODO: in function, free na getcwd(NULL, 0)

        // Check for exit command
        if(!strcmp(input, COMMAND_EXIT)) {
            break;
        }

        // Check for cd command
        if (strstr(input, COMMAND_CD) == input) {
            // TODO: 'cd -', 'cd'
            if (chdir(input + strlen(COMMAND_CD) + 1) == -1) {
                perror("Could not change directory");
            }
            continue;
        }

        if ((sink = strstr(input, COMMAND_PIPE)) != NULL) {  // TODO: use get_tokens
          sink[0] = 0;  // \0 after left argument of pipe
          sink = sink + strlen(COMMAND_PIPE);  // skip pipe delimiter
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
