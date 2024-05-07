#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200

/*
En este práctico, se busca el desarrollo de un shell interactivo en lenguaje C. Un shell, en esencia, es un programa que ejecuta otros programas en respuesta a 
comandos introducidos mediante texto. Dado que el objetivo principal es evaluar el conocimiento sobre aspectos fundamentales de sistemas operativos, se ha optado 
por omitir la tarea de procesar cadenas de texto. El parsing es crucial en un shell, ya que un algoritmo sólido para esta tarea simplifica considerablemente el 
diseño de la interacción entre el software y el sistema operativo.

Como seguramente ya estarán intuyendo, un shell es un software que actúa como interfaz con el sistema operativo, facilitando la interacción de los programas a nivel 
usuario con el SO. Esta interacción se lleva a cabo mediante syscalls (llamadas al sistema), es decir, cuando un programa requiere realizar una tarea que demanda 
privilegios de nivel de sistema operativo, como leer o escribir en el disco, enviar datos a través de la red o crear un nuevo proceso, no puede acceder directamente 
a los recursos del sistema. En su lugar, realiza una llamada al sistema, que consiste en una solicitud al kernel para que realice la operación en su nombre. 

La biblioteca GNU C encapsula estas llamadas al sistema mediante funciones que siguen el estándar POSIX2 que ya estuvieron utilizando, por ejemplo: fork, open, read, 
write, dup, dup2, waitpid, execvp, close, etc. Recuerden que ejecutando en su terminal el comando: man execvp
pueden obtener información sumamente útil y detallada sobre la funcionalidad de los diferentes syscalls. Otro punto importante, es que lean la documentación en el 
manual de linux de TODAS las syscalls que van a estar utilizando, sobre todo hagan hincapié en entender que hace waitpid y close para usarlas de manera correcta 
porque les va a ahorrar bastantes dolores de cabeza.

En concreto, el código que deben programar debe resolver el siguiente problema: dada una cadena de programas separados mediante pipes “|”, su programa debe poder 
reproducir la funcionalidad de su bash.
*/

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
                token = strtok(commands[i], ' ');
                while (token != NULL && arg_count < 255) {
                    if (token[0] == '\'' || token[0] == '\"') {
                        memmove(token, token + 1, strlen(token)); // Remove leading quote
                        if (token[strlen(token) - 1] == '\'' || token[strlen(token) - 1] == '\"') {
                            token[strlen(token) - 1] = '\0'; // Remove trailing quote
                        }
                    }
                    args[arg_count++] = token;
                    token = strtok(NULL, ' ');
                }
                args[arg_count] = NULL;
                // token = strtok(commands[i], " ");
                // while (token != NULL && arg_count < 255) {
                //     if (token[0] == '\'' || token[0] == '\"') {
                //         memmove(token, token + 1, strlen(token)); // Remove leading quote
                //         if (token[strlen(token) - 1] == '\'' || token[strlen(token) - 1] == '\"') {
                //             token[strlen(token) - 1] = '\0'; // Remove trailing quote
                //         }
                //     }
                //     args[arg_count++] = token;
                //     token = strtok(NULL, " ");
                // }
                // args[arg_count] = NULL;

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

//         int fds[command_count][2];
//         int pids[command_count];

//         for (int i = 0; i < command_count; i++) {
//             if (pipe(fds[i]) == -1) {
//                 perror("pipe");
//                 exit(EXIT_FAILURE);
//             }
//         }

//         for (int i = 0; i < command_count; i++) {
//             pids[i] = fork();
//             if (pids[i] == -1) {
//                 perror("fork");
//                 exit(EXIT_FAILURE);
//             } else if (pids[i] == 0) {  // Child process
//                 if (i != 0) {
//                     close(fds[i - 1][1]);
//                     if (dup2(fds[i - 1][0], STDIN_FILENO) == -1) {
//                         perror("dup2");
//                         exit(EXIT_FAILURE);
//                     }
//                     close(fds[i - 1][0]);
//                 }
//                 if (i != command_count - 1) {
//                     close(fds[i][0]);
//                     if (dup2(fds[i][1], STDOUT_FILENO) == -1) {
//                         perror("dup2");
//                         exit(EXIT_FAILURE);
//                     }
//                     close(fds[i][1]);
//                 }
//                 for (int j = 0; j < command_count; j++) {
//                     if (j != i) {
//                         close(fds[j][0]);
//                         close(fds[j][1]);
//                     }
//                 }

//                 char *args[256];
//                 int arg_count = 0;
//                 token = strtok(commands[i], " ");
//                 while (token != NULL && arg_count < 255) {
//                     if (token[0] == '\"') {
//                         token++;
//                     }
//                     if (token[strlen(token) - 1] == '\"') {
//                         token[strlen(token) - 1] = '\0';
//                     }
//                     args[arg_count++] = token;
//                     token = strtok(NULL, " ");
//                 }
//                 args[arg_count] = NULL;

//                 execvp(args[0], args);
//                 perror("execvp");
//                 fprintf(stderr, "Failed to execute command: %s\n", args[0]);
//                 exit(EXIT_FAILURE);
//             }
//         }

//         for (int i = 0; i < command_count; i++) {
//             close(fds[i][0]);
//             close(fds[i][1]);
//             waitpid(pids[i], NULL, 0);
//         }

//         command_count = 0; // Reset command count for next iteration
//     }

//     return 0;
// }



        // int fds[command_count][2];
        // int pids[command_count];

        // for (int i = 0; i < command_count; i++) {
        //     if (pipe(fds[i]) == -1) {
        //         perror("pipe");
        //         exit(EXIT_FAILURE);
        //     }
        // }

        // for (int i = 0; i < command_count; i++) {
        //     pids[i] = fork();
        //     if (pids[i] == -1) {
        //         perror("fork");
        //         exit(EXIT_FAILURE);
        //     } else if (pids[i] == 0) {  // Child process
        //         if (i != 0) {
        //             close(fds[i - 1][1]);
        //             if (dup2(fds[i - 1][0], STDIN_FILENO) == -1) {
        //                 perror("dup2");
        //                 exit(EXIT_FAILURE);
        //             }
        //             close(fds[i - 1][0]);
        //         }
        //         if (i != command_count - 1) {
        //             close(fds[i][0]);
        //             if (dup2(fds[i][1], STDOUT_FILENO) == -1) {
        //                 perror("dup2");
        //                 exit(EXIT_FAILURE);
        //             }
        //             close(fds[i][1]);
        //         }
        //         for (int j = 0; j < command_count; j++) {
        //             close(fds[j][0]);
        //             close(fds[j][1]);
        //         }
                
        //         token = strtok(commands[i], " ");
        //         char *args[256];
        //         int arg_count = 0;
        //         int i = 0;
        //         while ((token != NULL) && (i < MAX_COMMANDS)) {
        //             // saco las dos comillas de cada lado
        //             if (token[0] == '\"') {
        //                 token++;
        //             }
        //             if (token[strlen(token) - 1] == '\"') {
        //                 token[strlen(token) - 1] = '\0';
        //             }
        //             args[i++] = token;
        //             token = strtok(NULL, " "); 
        //         }
                
        //         args[i] = NULL;
        //         execvp(args[0], args);
        //         perror("execvp");
        //         exit(EXIT_FAILURE);
    
        //     }

        // }

        // for (int i = 0; i < command_count; i++) {
        //     close(fds[i][0]);
        //     close(fds[i][1]);
        //     waitpid(pids[i], NULL, 0);
        // }
    // }

        
    //     // Check if any commands were provided
    //     if (command_count == 0) {
    //         printf("No se ingresaron comandos.\n");
    //         continue;
    //     }

    //     // Add a NULL pointer to indicate the end of the commands array
    //     commands[command_count] = NULL;

    //     // Create pipes for inter-process communication
    //     int pipes[2 * (command_count - 1)];
    //     for (int i = 0; i < command_count - 1; i++) {
    //         if (pipe(pipes + i * 2) == -1) {
    //             perror("pipe");
    //             exit(EXIT_FAILURE);
    //         }
    //     }

    //     // Execute commands in child processes
    //     for (int i = 0; i < command_count; i++) {
    //         pid_t pid = fork();
    //         if (pid == -1) {
    //             perror("fork");
    //             exit(EXIT_FAILURE);
    //         } else if (pid == 0) {  // Child process
    //             // Set up input redirection
    //             if (i != 0) {
    //                 if (dup2(pipes[(i - 1) * 2], STDIN_FILENO) == -1) {
    //                     perror("dup2");
    //                     exit(EXIT_FAILURE);
    //                 }
    //             }
    //             // Set up output redirection
    //             if (i != command_count - 1) {
    //                 if (dup2(pipes[i * 2 + 1], STDOUT_FILENO) == -1) {
    //                     perror("dup2");
    //                     exit(EXIT_FAILURE);
    //                 }
    //             }
    //             // Close all pipe descriptors
    //             for (int j = 0; j < 2 * (command_count - 1); j++) {
    //                 close(pipes[j]);
    //             }
    //             // Execute the command
    //             execlp(commands[i], commands[i], NULL);
    //             // If execlp returns, there was an error
    //             perror("execlp");
    //             exit(EXIT_FAILURE);
    //         }
    //     }

    //     // Close all pipe descriptors in the parent process
    //     for (int i = 0; i < 2 * (command_count - 1); i++) {
    //         close(pipes[i]);
    //     }

    //     // Wait for all child processes to finish
    //     for (int i = 0; i < command_count; i++) {
    //         wait(NULL);
    //     }

    //     // Reset command count for the next iteration
    //     command_count = 0;
    // }

    // return 0;
    
// }
