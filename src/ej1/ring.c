#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{	
	int start, status, pid, n;
	int buffer[1];

	if (argc != 4){ printf("Uso: anillo <n> <c> <s> \n"); exit(0);}
    
    /* Parsing of arguments */
	n = atoi(argv[1]); // cantidad de procesos del anillo
	buffer[0] = atoi(argv[2]); // valor del mensaje inicial
	start = atoi(argv[3]); // número de proceso que inicia el comunicación
	
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);

	int fds[n][2];
	int fds_last[2];

    // Crear los pipes
    for (int i = 0; i < n; i++) {
        if (pipe(fds[i]) != 0) {
            fprintf(stderr, "Error en la creación de pipes");
            return (-1);
        }
    }

	if (pipe(fds_last) != 0) {
			fprintf(stderr, "Error en la creación de pipes");
            return (-1);
	}

	// Proceso padre envía el mensaje inicial al primer hijo
    printf("Proceso padre envía el mensaje %d al proceso %d\n", buffer[0], start);
    write(fds[start][1], &buffer, sizeof(int));

    // Crear procesos hijos
    for (int i = 0; i < n; i++) {
        pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Error en la creación del proceso");
            return (-1);
        }
        if (pid == 0) {  // Proceso hijo
            if (i != start) {
                read(fds[i][0], &buffer, sizeof(int));
            }
            printf("Proceso %d recibió el mensaje %d\n", i, buffer[0]);
            buffer[0]++; 
            if (i == (start + n - 1) % n) { // Último hijo
                printf("Proceso %d envía el mensaje %d al proceso padre\n", i, buffer[0]);
				write(fds_last[1], &buffer, sizeof(int));;
            } else {
                printf("Proceso %d envía el mensaje %d al siguiente proceso\n", i, buffer[0]);
                write(fds[(i + 1) % n][1], &buffer, sizeof(int)); 
            }

			for (int j = 0; j < n; j++) {
				close(fds[j][0]);
				close(fds[j][1]);
			}
			close(fds_last[0]);
			close(fds_last[1]);

			return 0;

        } else {  // Proceso padre
            if (i == (start + n - 1) % n) { // Si es el último proceso hijo, guardo su PID
				status = pid;
            }
        }
    }

    // Esperar a que el último proceso hijo termine
    waitpid(status, NULL, 0);

	// Leer el mensaje final del proceso padre
	printf("Proceso padre está a punto de leer el mensaje final\n");
	read(fds_last[0], &buffer, sizeof(int));
	printf("Proceso padre recibió el mensaje final %d\n", buffer[0]);
	close(fds_last[0]);
	close(fds_last[1]);

    // Cerrar los pipes
    for (int i = 0; i < n; i++) {
        close(fds[i][0]);
        close(fds[i][1]);
    }

    return 0;
}

