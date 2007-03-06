#define DEBUG 1

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


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


#define READ_BUFFER_SIZE 20
#define ENVIRONMENT_HOMEDIR "HOME"
#define DIRECTORY_SEPARATOR '/'
#define PROMPT_FORMAT "%s> "
#define COMMAND_SEPARATOR ' '
#define COMMAND_EXIT "exit"
#define COMMAND_CD "cd"
#define COMMAND_PIPE "|"

#define NO_PIPE 0
#define WRITE_TO_PIPE 1
#define READ_FROM_PIPE 2


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
int get_tokens (char *line, char delimiter, char **tokens, int tokens_length) {

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
  Execute the command with possible parameter.
*/
void execute_command (char **arguments, int *fd, int pipe_action) {

    pid_t pid;
    char *command;

    pid = fork();

    if (pid < 0) {
        perror("Unable to fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // child process

        if (arguments[0] == NULL || strlen(arguments[0]) == 0) {
            exit(EXIT_FAILURE);
        }

        // Store executable command
        command = arguments[0];

        // Skip preceding path for executable name
        if(strrchr(arguments[0], DIRECTORY_SEPARATOR) != NULL) {
            arguments[0] = strrchr(arguments[0], DIRECTORY_SEPARATOR) + 1;
        }

        if (pipe_action == WRITE_TO_PIPE) {
            if (close(fd[0]) == -1) {
                perror(NULL);
                exit(EXIT_FAILURE);
            }
            if (close(STDOUT_FILENO) == -1) {
                perror(NULL);
                exit(EXIT_FAILURE);
            }
            if (dup2(fd[1], STDOUT_FILENO) == -1) {
                perror(NULL);
                exit(EXIT_FAILURE);
            }
        }

        if (pipe_action == READ_FROM_PIPE) {
            if (close(fd[1]) == -1) {
                perror(NULL);
                exit(EXIT_FAILURE);
            }
            if (close(STDIN_FILENO) == -1) {
                perror(NULL);
                exit(EXIT_FAILURE);
            }
            if (dup2(fd[0], STDIN_FILENO) == -1) {
                perror(NULL);
                exit(EXIT_FAILURE);
            }
        }

        execvp(command, arguments);
        perror(command);
        exit(EXIT_FAILURE);
    }

}


/*
  Print the prompt and read a line of input.
  Return this line or NULL in case of EOF.
*/
char *read_prompt() {

    char *buffer;
    int buffer_size;
    char *p; // pointer to current character in buffer
    char *current_working_dir;

    int character;
    int length = 0;

    // Print prompt
    current_working_dir = getcwd(NULL, 0);
    printf(PROMPT_FORMAT, current_working_dir);
    free(current_working_dir);

    buffer = malloc(READ_BUFFER_SIZE);

    if (buffer == NULL) {
        perror("Unable to allocate necessary memory");
        exit(EXIT_FAILURE);
    }

    buffer_size = READ_BUFFER_SIZE;
    p = buffer;

    while (1) {
        character = fgetc(stdin);

        if (character == EOF) {
            if (length > 0) { // we have a string to return
                *p = '\0';
            } else { // no string, return null
                free(buffer);
                buffer = NULL;
            }
            break;
        } else if (character == '\n') {
            *p = '\0';
            break;
        }
        /*
          If character == '\0' we read more than we process, but that's not
          too bad I guess.
        */

        *p = character; // put character in buffer
        p++;
        length++;

        // check if we need to resize the buffer
        if (length >= buffer_size) {
            buffer_size += READ_BUFFER_SIZE;
            buffer = realloc(buffer, buffer_size);
            if (buffer == NULL) {
                /*
                  Without the exit() we should do a free() on the original
                  buffer (this would require some more lines though, because
                  we just lost the pointer to it...)
                */
                perror("Unable to allocate necessary memory");
                exit(EXIT_FAILURE);
            }
            p = buffer + length;
        }

    }

    // Go to a new line on EOF
    if (!buffer) {
        printf("\n");
    }

    return buffer;
}


/*
  Check for an exit command. Return 1 on succes, 0 otherwise.
*/
int do_exit(char **arguments) {

    if ((arguments[0] != NULL) && !strcmp(arguments[0], COMMAND_EXIT)) {
        return 1;
    }

    return 0;
}


/*
  Check for a cd command and possible directory parameter and change the current
  working directory accordingly. Return 0 on success, non-zero otherwise.
*/
int do_cd(char **arguments) {

    char *homedir;

    if ((arguments[0] != NULL) && !strcmp(arguments[0], COMMAND_CD)) {
        if (arguments[1] != NULL) {
            if (chdir(arguments[1]) == -1) {
                perror("Could not change directory");
            }
        } else {
            homedir = getenv(ENVIRONMENT_HOMEDIR);
            if (homedir) {
                if (chdir(homedir) == -1) {
                    perror("Could not change directory");
                }
            } else {
                printf("Could not read %s environment variable\n",
                       ENVIRONMENT_HOMEDIR);
            }
        }
        return 1;
    }

    return 0;
}


/*
  Check the command for the presence of a pipe. If it is there, truncate the
  origininal arguments list at that point and return a pointer to the rest. Also
  create a pipe.
*/
char **create_pipe(char **arguments, int *fd) {

    int i;
    char **sink_arguments = NULL;

    // Find a pipe command
    for (i = 0; arguments[i] != NULL; i++) {

        if (!strcmp(arguments[i], COMMAND_PIPE)) {

            // Truncate original argument list
            arguments[i] = NULL;

            // Next argument is start of sink arguments
            sink_arguments = &arguments[i + 1];

            // Create pipe
            if (pipe(fd) < 0) {
                perror("Cannot create pipe");
                exit(EXIT_FAILURE);
            }

            break;
        }
    }

    return sink_arguments;
}


int main(int argc, char **argv) {

    char *input;
    int fd[2];
    int num_arguments;
    char **arguments;
    char **sink_arguments = NULL;

    while ((input = read_prompt())) {

        num_arguments = num_tokens(input, COMMAND_SEPARATOR);

        /*
          Allocate array of pointers to arguments with a size of num_arguments
          + 1 because we end it with a NULL pointer.
          Actually, this is only because it makes it easy to call execvp().
        */
        arguments = (char **) malloc(sizeof(char *) * (num_arguments + 1));

        if (arguments == NULL) {
            perror("Unable to allocate necessary memory");
            exit(EXIT_FAILURE);
        }

        get_tokens(input, COMMAND_SEPARATOR, arguments, num_arguments);

        // End argument list with NULL pointer
        arguments[num_arguments] = (char *) NULL;

        if (do_exit(arguments)) {
            break;
        }

        if (do_cd(arguments)) {
            continue;
        }

        // TODO: seriously check if this refactoring of fork() inside
        // execute_command is OK (returns, waits, etc)

        if ((sink_arguments = create_pipe(arguments, fd)) != NULL) {

            execute_command(arguments, fd, WRITE_TO_PIPE);
            execute_command(sink_arguments, fd, READ_FROM_PIPE);
            close(fd[0]);
            close(fd[1]);
            wait((void *) NULL);

        } else {
            execute_command(arguments, fd, NO_PIPE);
        }

        wait((void *) NULL);
        free(input);
    }

    exit(EXIT_SUCCESS);

}
