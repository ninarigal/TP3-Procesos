#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/*
Como warm up para este primer ejercicio, el objetivo es implementar un esquema de comunicación en forma de anillo para interconectar los procesos. En un esquema de 
anillo se da con al menos tres procesos están conectados formando un bucle cerrado. Cada proceso está comunicado exactamente con dos procesos: su predecesor y su 
sucesor. Recibe un mensaje del predecesor y lo envía al sucesor. En este caso, la comunicación se llevará a cabo a través de pipes, las cuales deben ser 
implementadas.
Al inicio, alguno de los procesos del anillo recibirá un número entero como mensaje a transmitir. Este mensaje será enviado al siguiente proceso en el anillo, 
quien, tras recibirlo, lo incrementará en uno y luego lo enviará al siguiente proceso en el anillo. Este proceso continuará hasta que el proceso que inició la 
comunicación reciba, del último proceso, el resultado del mensaje inicialmente enviado.
Se sugiere que el programa inicial cree un conjunto de procesos hijos, que deben ser organizados para formar un anillo. Por ejemplo, el hijo 1 recibe el mensaje, lo 
incrementa y lo envía al hijo 2. Este último lo incrementa nuevamente y lo pasa al hijo 3, y así sucesivamente, hasta llegar al último hijo, que incrementa el valor 
por última vez y lo envía de vuelta al proceso padre. Este último debe mostrar el resultado final del proceso de comunicación en la salida estándar.
*/


int main(int argc, char **argv)
{	
	int start, status, pid, n, last_child_pid;
	int buffer[1];

	if (argc != 4){ printf("Uso: anillo <n> <c> <s> \n"); exit(0);}
    
    /* Parsing of arguments */
  	/* TO COMPLETE */
	n = atoi(argv[1]); // cantidad de procesos del anillo
	buffer[0] = atoi(argv[2]); // valor del mensaje inicial
	start = atoi(argv[3]); // número de proceso que inicia el comunicación
	
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);


// 	int pipes[n][2];

//     // Crear los pipes
//     for (int i = 0; i < n; i++) {
//         if (pipe(pipes[i]) == -1) {
//             perror("Error en la creación de pipes");
//             exit(EXIT_FAILURE);
//         }
//     }

//     // Crear procesos hijos
//     for (int i = 0; i < n; i++) {
//         pid = fork();

//         if (pid < 0) {
//             perror("Error en la creación del proceso");
//             exit(EXIT_FAILURE);
//         }

//         if (pid == 0) {  // Proceso hijo
//             close(pipes[i][1]);  // Cerramos el extremo de escritura

//             if (i != start) { // Los hijos no inician el mensaje
//                 read(pipes[i][0], &buffer, sizeof(int));
//                 // printf("Proceso %d recibió el mensaje %d\n", i, buffer[0]);
//             }
// 			printf("Proceso %d recibió el mensaje %d\n", i, buffer[0]);
// 			buffer[0]++; // Incrementamos el mensaje

// 			if (i == (start + n - 1) % n) { // El último hijo envía el mensaje al padre
// 				printf("Proceso %d envía el mensaje %d al proceso padre\n", i, buffer[0]);
// 				write(pipes[start][1], &buffer, sizeof(int)); // Enviamos el mensaje al proceso padre
// 				close(pipes[i][0]);  
// 				close(pipes[start][1]);
// 				exit(EXIT_SUCCESS);
// 			} else {
// 				printf("Proceso %d envía el mensaje %d al siguiente proceso\n", i, buffer[0]);
// 				write(pipes[(i + 1) % n][1], &buffer, sizeof(int)); // Enviamos el mensaje al siguiente proceso
// 				close(pipes[i][0]);  
// 				close(pipes[(i + 1) % n][1]);  
// 				exit(EXIT_SUCCESS);

// 			}
//         } else {  // Proceso padre
//             close(pipes[i][0]);  // Cerramos el extremo de lectura
//         }
//     }

//     // Proceso padre envía el mensaje inicial al primer hijo
//     printf("Proceso padre envía el mensaje %d al proceso %d\n", buffer[0], start);
//     write(pipes[start][1], &buffer, sizeof(int));
//     close(pipes[start][1]);  // Cerramos el extremo de escritura

// 	// Leer el mensaje final del proceso padre
// 	read(pipes[start][0], &buffer, sizeof(int));
// 	printf("Proceso padre recibió el mensaje %d\n", buffer[0]);
// 	close(pipes[start][0]);  // Cerramos el extremo de lectura

// 	// Esperar a que los procesos hijos terminen
// 	for (int i = 0; i < n; i++) {
// 		waitpid(-1, &status, 0);
// 	}

// 	// Cerrar los pipes
// 	for (int i = 0; i < n; i++) {
// 		close(pipes[i][0]);
// 		close(pipes[i][1]);
// 	}

// 	return 0;
// }

	int fds[n][2];

    // Crear los pipes
    for (int i = 0; i < n; i++) {
        if (pipe(fds[i]) == -1) {
            perror("Error en la creación de pipes");
            exit(EXIT_FAILURE);
        }
    }

    // Crear procesos hijos
    for (int i = 0; i < n; i++) {
        pid = fork();

        if (pid < 0) {
            perror("Error en la creación del proceso");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {  // Proceso hijo
            close(fds[i][1]);  // Cerramos el extremo de escritura

            if (i != start) { // Los hijos no inician el mensaje
                read(fds[i][0], &buffer, sizeof(int));
            }
            printf("Proceso %d recibió el mensaje %d\n", i, buffer[0]);
            buffer[0]++; // Incrementamos el mensaje

            if (i == (start + n - 1) % n) { // El último hijo envía el mensaje al proceso padre
                printf("Proceso %d envía el mensaje %d al proceso padre\n", i, buffer[0]);
                write(fds[start][1], &buffer, sizeof(int)); // Enviamos el mensaje al proceso padre
                close(fds[i][0]);
                exit(EXIT_SUCCESS);
            } else {
                printf("Proceso %d envía el mensaje %d al siguiente proceso\n", i, buffer[0]);
                write(fds[(i + 1) % n][1], &buffer, sizeof(int)); // Enviamos el mensaje al siguiente proceso
                close(fds[i][0]);
                close(fds[(i + 1) % n][1]);
                exit(EXIT_SUCCESS);
            }
        } else {  // Proceso padre
            if (i == (i + 1) % n) { // Si es el último proceso hijo, guardamos su PID
				last_child_pid = pid;
            }
            close(fds[i][0]);  // Cerramos el extremo de lectura
        }
    }

    // Proceso padre envía el mensaje inicial al primer hijo
    printf("Proceso padre envía el mensaje %d al proceso %d\n", buffer[0], start);
    write(fds[start][1], &buffer, sizeof(int));
    close(fds[start][1]);  // Cerramos el extremo de escritura

    // Esperar a que el último proceso hijo termine
    waitpid(last_child_pid, NULL, 0);

    // Leer el mensaje final del proceso padre
    printf("Proceso padre está a punto de leer el mensaje final\n");
    read(fds[start][0], &buffer, sizeof(int));
    printf("Proceso padre recibió el mensaje final %d\n", buffer[0]);
    close(fds[start][0]);

    // Cerrar los pipes
    for (int i = 0; i < n; i++) {
        close(fds[i][0]);
        close(fds[i][1]);
    }

    return 0;
}

