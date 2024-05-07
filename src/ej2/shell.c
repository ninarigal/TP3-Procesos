#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200

char *strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *p = malloc(len);
    if (p != NULL) {
        memcpy(p, s, len);
    }
    return p;
}

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
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < command_count; i++) {
            pids[i] = fork();
            if (pids[i] == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pids[i] == 0) {  // Child process
                if (i != 0) {
                    close(fds[i - 1][1]);
                    if (dup2(fds[i - 1][0], STDIN_FILENO) == -1) {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                    close(fds[i - 1][0]);
                }
                if (i != command_count - 1) {
                    close(fds[i][0]);
                    if (dup2(fds[i][1], STDOUT_FILENO) == -1) {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                    close(fds[i][1]);
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
                int in_quotes = 0;
                char arg_buffer[256] = ""; // Buffer para almacenar el argumento entre comillas

                while (token != NULL && arg_count < 255) {
                    if (token[0] == '\'' || token[0] == '\"') {
                        if (token[strlen(token) - 1] == '\'' || token[strlen(token) - 1] == '\"') {
                            memmove(token, token + 1, strlen(token)); // Remove leading quote
                            token[strlen(token) - 1] = '\0'; // Remove trailing quote
                            args[arg_count++] = strdup(token);
                        } else {
                            in_quotes = 1;
                            strcpy(arg_buffer, token + 1); // Copiar el token sin la comilla inicial
                        }
                    } else if (in_quotes && (token[strlen(token) - 1] == '\'' || token[strlen(token) - 1] == '\"')) {
                        strcat(arg_buffer, " "); // Agregar espacio si es necesario
                        strncat(arg_buffer, token, strlen(token) - 1); // Concatenar el token sin la comilla final
                        args[arg_count++] = strdup(arg_buffer); // Agregar el argumento completo al array de argumentos
                        in_quotes = 0; // Restablecer el estado de comillas
                        arg_buffer[0] = '\0'; // Reiniciar el buffer
                    } else {
                        if (in_quotes) {
                            strcat(arg_buffer, " "); // Agregar espacio si es necesario
                            strcat(arg_buffer, token); // Concatenar el token al buffer
                        } else {
                            args[arg_count++] = strdup(token); // Agregar el token al array de argumentos
                        }
                    }
                    token = strtok(NULL, " ");
                    
                }
                args[arg_count] = NULL;

                execvp(args[0], args);
                perror("execvp");
                fprintf(stderr, "Failed to execute command: %s\n", args[0]);
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < command_count; i++) {
            close(fds[i][0]);
            close(fds[i][1]);
            waitpid(pids[i], NULL, 0);
        }

        command_count = 0; // Reset command count for next iteration
    }

    return 0;
}
