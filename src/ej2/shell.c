#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200

int main() {

    char command[256];
    char *commands[MAX_COMMANDS];
    int command_count = 0;

    while (1) 
    {
        printf("Shell> ");
        
        /*Reads a line of input from the user from the standard input (stdin) and stores it in the variable command */
        fgets(command, sizeof(command), stdin);
        
        /* Removes the newline character (\n) from the end of the string stored in command, if present. 
           This is done by replacing the newline character with the null character ('\0').
           The strcspn() function returns the length of the initial segment of command that consists of 
           characters not in the string specified in the second argument ("\n" in this case). */
        command[strcspn(command, "\n")] = '\0';

        /* Tokenizes the command string using the pipe character (|) as a delimiter using the strtok() function. 
           Each resulting token is stored in the commands[] array. 
           The strtok() function breaks the command string into tokens (substrings) separated by the pipe character |. 
           In each iteration of the while loop, strtok() returns the next token found in command. 
           The tokens are stored in the commands[] array, and command_count is incremented to keep track of the number of tokens found. */
        char *token = strtok(command, "|");
        while (token != NULL) 
        {
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }
        /* You should start programming from here... */
        int fds[command_count][2];
        int pids[command_count];

        for (int i = 0; i < command_count; i++) {
            if (pipe(fds[i]) == -1) {
                fprintf(stderr, "Error en la creación de pipes");
                return (-1);
            }
        }

        for (int i = 0; i < command_count; i++) {
            pids[i] = fork();
            if (pids[i] == -1) {
                fprintf(stderr, "Error en la creación del proceso");
                return (-1);
            } else if (pids[i] == 0) {  // Child process
                if (i != 0) {
                    if (dup2(fds[i - 1][0], STDIN_FILENO) == -1) {
                        fprintf(stderr, "Error en dup2");
                        return (-1);
                    }
                }
                if (i != command_count - 1) {
                    if (dup2(fds[i][1], STDOUT_FILENO) == -1) {
                        fprintf(stderr, "Error en dup2");
                        return (-1);
                    }
                }
                for (int j = 0; j < command_count; j++) {
                    if (j != i) {
                        close(fds[j][0]);
                        close(fds[j][1]);
                    }
                }

                char *args[256];
                int arg_count = 0;
                token = strtok(commands[i], " ");
                while (token != NULL && arg_count < 255) {
                    if (token[0] == '\'' || token[0] == '\"') {
                        memmove(token, token + 1, strlen(token)); // Remove leading quote
                        if (token[strlen(token) - 1] == '\'' || token[strlen(token) - 1] == '\"') {
                            token[strlen(token) - 1] = '\0'; // Remove trailing quote
                        }
                    }
                    args[arg_count++] = token;
                    token = strtok(NULL, " ");
                }
                args[arg_count] = NULL; 
                free(commands[i]);
                free(token);

                execvp(args[0], args);
                fprintf(stderr, "Failed to execute command: %s\n", args[0]);
                return (-1);
            }
        }

        for (int i = 0; i < command_count; i++) {
            close(fds[i][0]);
            close(fds[i][1]);
            waitpid(pids[i], NULL, 0);
        }

        command_count = 0; 
    }

    return 0;
}
