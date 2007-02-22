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


#define PROMPT_FORMAT "%s> "
#define PROMPT_MAX_LENGTH 100
#define DIRECTORY_SEPARATOR '/'
#define COMMAND_SEPARATOR ' '
#define COMMAND_EXIT "exit"
#define COMMAND_CD "cd"
#define COMMAND_PIPE "|"


/*
  Count number of tokens separated by one or more delimiter occurrences.
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
  Get tokens separated by one or more delimiter occurrences.
  Original line will be modified!
  Return number of tokens read (max of tokens_length).
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
  Execute the command with possible parameters separated by spaces.
*/
int exec_command (char *line) {

    char **arguments;
    char *command;
    int num_arguments;

    if (!line) {
        return -1;
    }

    num_arguments = num_tokens(line, COMMAND_SEPARATOR);

    if (num_arguments < 1) {
        return -1;
    }

    // Allocate array of pointers to arguments
    arguments = (char **) malloc(sizeof(char *) * (num_arguments + 1));

    get_tokens(line, COMMAND_SEPARATOR, arguments, num_arguments);

    // End argument list with NULL pointer
    arguments[num_arguments] = (char *) NULL;

    // Store executable command
    command = arguments[0];

    // Skip preceding path for executable name
    if(strrchr(arguments[0], DIRECTORY_SEPARATOR) != NULL) {
        arguments[0] = strrchr(arguments[0], DIRECTORY_SEPARATOR) + 1;
    }

    execvp(command, arguments);

    // Only reached on execvp error
    perror(command);
    free(arguments);

    return -1;

}


/*
  Print the prompt and read a line of input.
  Return this line or NULL in case of EOF.
*/
char *read_prompt() {

    char prompt[PROMPT_MAX_LENGTH];
    char *current_working_dir;

    current_working_dir = getcwd(NULL, 0);
    snprintf(prompt, sizeof(prompt), PROMPT_FORMAT, current_working_dir);
    free(current_working_dir);

    return readline(prompt);

}


int do_exit(char *command) {
    return !strcmp(command, COMMAND_EXIT);
}


/*
  Check for a cd command and possible directory parameter and change the current
  working directory accordingly.
*/
int do_cd(char *command) {

    char *tokenized_command;
    char *tokens[2];
    int num_tokens;

    tokenized_command = (char *) malloc(strlen(command) + 1);
    strcpy(tokenized_command, command);

    num_tokens = get_tokens(tokenized_command, COMMAND_SEPARATOR, tokens, 2);

    if ((num_tokens > 0) && !strcmp(tokens[0], COMMAND_CD)) {
        if (num_tokens > 1) {
            if (chdir(tokens[1]) == -1) {
                perror("Could not change directory");
            }
        } else {
            // TODO: cd to home dir
            printf("The cd command expects one argument\n");
        }
        free(tokenized_command);
        return 1;
    }


    free(tokenized_command);
    return 0;

}


/*
  Check the command for the presence of a pipe. If it is there, return the
  location of the sink argument and create a pipe.
*/
char *do_pipe(char *command, int *fd) {

    char *sink;

    // Locate pipe character
    if ((sink = strstr(command, COMMAND_PIPE)) != NULL) {

        // End left argument of pipe with \0
        *sink = 0;

        // Skip pipe delimiter
        sink = sink + strlen(COMMAND_PIPE);

        // Create pipe
        if (pipe(fd) < 0) {
            perror("Could not create pipe");
            exit(1);
        }

        return sink;

    }

    return NULL;

}


int main(int argc, char **argv) {

    pid_t pid;
    pid_t pid2;
    char *input;
    char *sink;
    int piped_command = 0;
    int fd[2];

    while ((input = read_prompt()) != NULL) {

        if (do_exit(input)) {
            break;
        }

        if (do_cd(input)) {
            continue;
        }

        if ((sink = do_pipe(input, fd)) != NULL) {
            piped_command = 1;
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
              waitpid(pid2, (void *) NULL, 0); // wait for child to exit
              waitpid(pid, (void *) NULL, 0); // wait for child to exit

            }

          }

          if(input) {
            free(input);
            input = NULL;
          }

        }

    }

    exit(0);
}
