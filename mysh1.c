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
#define DIRECTORY_SEPARATOR '/'
#define COMMAND_EXIT "exit"


/*
  Execute the command with possible parameter.
*/
void execute_command (char **arguments) {

    pid_t pid;
    char *command;

    if (arguments[0] == NULL) {
        return;
    }

    // Store executable command
    command = arguments[0];

    // Skip preceding path for executable name
    if(strrchr(arguments[0], DIRECTORY_SEPARATOR) != NULL) {
        arguments[0] = strrchr(arguments[0], DIRECTORY_SEPARATOR) + 1;
    }

    pid = fork();

    if (pid < 0) {
        perror("Unable to fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // child process
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

    char *buffer; //[READ_BUFFER_SIZE];
    int buffer_size;
    char *p; // pointer to current character in buffer

    int character;
    int length = 0;

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
                  TODO: free() original buffer, but we just lost the pointer.
                  We can solve this by p=realloc() etcetera...
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


int main(int argc, char **argv) {

    char *input;
    char **arguments;

    while ((input = read_prompt())) {


        /*
          Allocate array of pointers to arguments with a size of 1 + 1 because
          there will be 1 argument (the program) and we end with a NULL pointer.
          Actually, this is only because it makes it easy to call execvp().
        */
        arguments = (char **) malloc(sizeof(char *) * (1 + 1));

        if (arguments == NULL) {
            perror("Unable to allocate necessary memory");
            exit(EXIT_FAILURE);
        }

        arguments[0] = input;

        // End argument list with NULL pointer
        arguments[1] = (char *) NULL;

        if (do_exit(arguments)) {
            break;
        }

        execute_command(arguments);
 
        wait((void *) NULL);
        free(input);
    }

    exit(EXIT_SUCCESS);

}
