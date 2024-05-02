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
	int start, status, pid, n;
	int buffer[1];

	if (argc != 4){ printf("Uso: anillo <n> <c> <s> \n"); exit(0);}
    
    /* Parsing of arguments */
  	/* TO COMPLETE */
	n = atoi(argv[1]); // cantidad de procesos del anillo
	buffer[0] = atoi(argv[2]); // valor del mensaje inicial
	start = atoi(argv[3]); // número de proceso que inicia el comunicación
	
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);
    
   	/* You should start programming from here... */

	// Creación de pipes
    int fd[n][2]; // file descriptors
    int i;
    for (i = 0; i < n; i++){
        if (pipe(fd[i]) != 0){
            fprintf(stderr, "Error al crear pipe\n");
            exit(1);
        }
    }

    // Creación de procesos
    for (i = 1; i <= n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Error al crear proceso\n");
            exit(1);
        } else if (pid == 0) {
            // Proceso hijo
            if (i == start) {
                // Si es el proceso inicial, envía el mensaje inicial
                write(fd[i-1][1], buffer, sizeof(buffer));
            }
            // Si no es el proceso inicial, recibe, incrementa y envía el mensaje
            read(fd[(i-1)%n][0], buffer, sizeof(buffer));
            buffer[0]++;
            write(fd[i%n][1], buffer, sizeof(buffer));
            exit(0);
        }
    }

    // Proceso padre
    int status;
    waitpid(-1, &status, 0);
    read(fd[0][0], buffer, sizeof(buffer));
    printf("El valor final es: %i\n", buffer[0]);

    return 0;
}

