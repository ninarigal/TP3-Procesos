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

	// int pipes[n][2];
    // pid_t pid;

    // for (int i = 0; i < n; i++) {
    //     pipe(pipes[i]);
    //     pid = fork();

    //     if (pid < 0) {
    //         perror("Error en la creación del proceso");
    //         exit(EXIT_FAILURE);
    //     }

    //     if (pid == 0) {  // Proceso hijo
    //         close(pipes[i][1]);  // Cerramos el extremo de escritura

    //         // Paso 4: Transmisión del mensaje
    //         // if (i == start) { 
	// 		// 	read(pipes[i][0], &buffer, sizeof(int));
	// 		// 	printf("Proceso %d recibió el mensaje %d\n", i, buffer[0]);
	// 		// 	buffer[0]++;
	// 		// } else {
	// 		// 	read(pipes[i][0], &buffer, sizeof(int));
	// 		// 	printf("Proceso %d recibió el mensaje %d\n", i, buffer[0]);
	// 		// 	buffer[0]++;
	// 		// }

	// 		read(pipes[i][0], &buffer, sizeof(int));
	// 		printf("Proceso %d recibió el mensaje %d\n", i, buffer[0]);
	// 		buffer[0]++;

	// 		// Corregir la condición para identificar el proceso que envía el mensaje de vuelta al padre
	// 		if (i == (start + n - 1) % n) {
	// 			printf("Proceso %d envía el mensaje %d al proceso padre\n", i, buffer[0]);
	// 			close(pipes[i][0]);  
	// 			write(pipes[start][1], &buffer, sizeof(int)); // Envía el mensaje al proceso padre
	// 			exit(EXIT_SUCCESS);
	// 		} else {
	// 			printf("Proceso %d envía el mensaje %d al siguiente proceso\n", i, buffer[0]);
	// 			write(pipes[(i + 1) % n][1], &buffer, sizeof(int));
	// 			close(pipes[i][0]);  
	// 			close(pipes[(i + 1) % n][1]);  
	// 			exit(EXIT_SUCCESS);
	// 		}
    //     } else {  // Proceso padre
    //         close(pipes[i][0]);  // Cerramos el extremo de lectura
    //     }
    // }

    // // Paso 3: Establecimiento de pipes
    // // Paso 4: Transmisión del mensaje (Proceso inicial)
    // printf("Proceso padre envía el mensaje %d al proceso %d\n", buffer[0], start);
    // write(pipes[start][1], &buffer, sizeof(int));

    // // Paso 5: Finalización de la comunicación (Proceso inicial)
    // for (int i = 0; i < n; i++) {
	// 	// wait(NULL);
	// 	close(pipes[i][1]);  // Cerramos el extremo de escritura
	// }

	// for (int i = 0; i < n; i++) {
	// 	wait(NULL);
	// 	// close(pipes[i][1]);  // Cerramos el extremo de escritura
	// }
    // // close(pipes[start][1]);  // Cerramos el extremo de escritura

	// // Paso 6: Finalización de la comunicación (Proceso padre)
	// read(pipes[start][0], &buffer, sizeof(int));
	// printf("El valor final es: %d\n", buffer[0]);
	// close(pipes[start][0]);  // Cerramos el extremo de lectura

    // return 0;

	int pipes[n][2];
    pid_t pid;

    for (int i = 0; i < n; i++) {
        pipe(pipes[i]);
        pid = fork();

        if (pid < 0) {
            perror("Error en la creación del proceso");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {  // Proceso hijo
            for (int j = 0; j < n; j++) {
                close(pipes[j][1]);  // Cerramos el extremo de escritura
            }

            read(pipes[i][0], &buffer, sizeof(int));
            printf("Proceso %d recibió el mensaje %d\n", i, buffer[0]);
            buffer[0]++; // Incrementamos el mensaje

            printf("Proceso %d envía el mensaje %d al siguiente proceso\n", i, buffer[0]);
            write(pipes[(i + 1) % n][1], &buffer, sizeof(int)); // Enviamos el mensaje al siguiente proceso
            close(pipes[i][0]);  
            close(pipes[(i + 1) % n][1]);  
            exit(EXIT_SUCCESS);
        } else {  // Proceso padre
            close(pipes[i][0]);  // Cerramos el extremo de lectura
        }
    }

    for (int i = 0; i < n; i++) {
        close(pipes[i][1]);  // Cerramos el extremo de escritura
    }

    int final_value = 0;
    for (int i = 0; i < n; i++) {
        read(pipes[i][0], &buffer, sizeof(int));
        final_value += buffer[0]; // Acumulamos el mensaje de todos los procesos
        close(pipes[i][0]);  // Cerramos el extremo de lectura
    }

    printf("El valor final es: %d\n", final_value);

    for (int i = 0; i < n; i++) {
        wait(NULL);
    }

    return 0;

}

// Para compilar: gcc -o ring ring.c
// Para ejecutar: ./ring <n> <c> <s> (n: cantidad de procesos, c: valor del mensaje inicial, s: número de proceso que inicia la comunicación)
// Ejemplo: ./ring 5 3 2

// Resultado esperado: El valor final es: 8
