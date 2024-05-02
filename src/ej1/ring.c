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
	int start, status, n;
	int buffer[1];

	if (argc != 4){ printf("Uso: anillo <n> <c> <s> \n"); exit(0);}
    
    /* Parsing of arguments */
  	/* TO COMPLETE */
	n = atoi(argv[1]); // cantidad de procesos del anillo
	buffer[0] = atoi(argv[2]); // valor del mensaje inicial
	start = atoi(argv[3]); // número de proceso que inicia el comunicación
	
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);
    
   	/* You should start programming from here... */
	/* TO COMPLETE */

	int fd[n][2];
    pid_t pids[n];

    // Crear pipes
    for (int i = 0; i < n; i++) {
        if (pipe(fd[i]) != 0) {
            fprintf(stderr, "Error en pipe\n");
            return -1;
        }
    }

    // Crear procesos hijos
    // for (int i = 0; i < n; i++) {
    //     pids[i] = fork();
    //     if (pids[i] == -1) {
    //         fprintf(stderr, "Error en fork\n");
    //         return -1;
    //     } else if (pids[i] == 0) { // Proceso hijo
    //         close(fd[i][1]); // Cerrar el descriptor de escritura en el proceso hijo
    //         read(fd[i][0], buffer, sizeof(buffer));
    //         printf("Proceso %d recibió el mensaje: %d\n", i, buffer[0]);
    //         buffer[0]++;
    //         printf("Proceso %d incrementó el mensaje a: %d\n", i, buffer[0]);
    //         write(fd[(i + 1) % n][1], buffer, sizeof(buffer));
    //         printf("Proceso %d envió el mensaje al siguiente proceso\n", i);
    //         close(fd[i][0]); // Cerrar el descriptor de lectura en el proceso hijo
    //         close(fd[(i + 1) % n][1]); // Cerrar el descriptor de escritura en el siguiente proceso
    //         return 0; // Importante: terminar el proceso hijo después de completar su trabajo
    //     }
    // }

	for (int i = 0; i < n; i++) {
		pids[i] = fork();
		if (pids[i] == -1) {
			fprintf(stderr, "Error en fork\n");
			return -1;
		} else if (pids[i] == 0) { // Proceso hijo
			close(fd[i][1]); // Cerrar el descriptor de escritura en el proceso hijo
			read(fd[i][0], buffer, sizeof(buffer));
			printf("Proceso %d recibió el mensaje: %d\n", i, buffer[0]);
			buffer[0]++;
			printf("Proceso %d incrementó el mensaje a: %d\n", i, buffer[0]);
			write(fd[(i + 1) % n][1], buffer, sizeof(buffer));
			printf("Proceso %d envió el mensaje al siguiente proceso\n", i);
			close(fd[i][0]); // Cerrar el descriptor de lectura en el proceso hijo
		}
	}

    // Proceso padre
    for (int i = 0; i < n; i++) {
        close(fd[i][0]); // Cerrar el descriptor de lectura en el proceso padre
        close(fd[i][1]); // Cerrar el descriptor de escritura en el proceso padre
    }

    // Iniciar la comunicación después de que se hayan creado todos los procesos hijos
	for (int i = 0; i < n; i++) {
    	if (i == start) { // Si es el proceso que inicia la comunicación, leer el mensaje inicial
			read(fd[start][0], &buffer, sizeof(buffer));
			printf("Proceso %d recibió el mensaje inicial: %d\n", start, buffer[0]);
    }
	}

	// Enviar el mensaje inicial al proceso que inicia la comunicación
	printf("Proceso padre envía el mensaje inicial: %d\n", buffer[0]);
	write(fd[start][1], &buffer, sizeof(buffer));


    // Esperar a que terminen todos los procesos hijos
    for (int i = 0; i < n; i++) {
        waitpid(pids[i], &status, 0);
    }

    printf("El valor final es: %d\n", buffer[0]);

    return 0;
}

// Para compilar: gcc -o ring ring.c
// Para ejecutar: ./ring <n> <c> <s> (n: cantidad de procesos, c: valor del mensaje inicial, s: número de proceso que inicia la comunicación)
// Ejemplo: ./ring 5 3 2

// Resultado esperado: El valor final es: 8
