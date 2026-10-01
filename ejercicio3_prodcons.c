/*
 * Ejercicio 3: Problema Productor-Consumidor con Búfer Acotado
 * Basado en la Figura 5.13 de Stallings (Sistemas Operativos)
 *
 * Compilacion:
 * gcc -o ejercicio3_prodcons ejercicio3_prodcons.c -lpthread -lrt
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define CAPACITY 5          // Tamanio del bufer acotado (Figura 5.13 Stallings)
#define NUM_PRODUCERS 3     // Identificadores de productores (P0, P1, P2)
#define NUM_CONSUMERS 3     // Identificadores de consumidores (C0, C1, C2)
#define ITEMS_PER_PROD 4    // Numero de items que produce cada hilo

// Bufer circular compartido
int buffer[CAPACITY];
int in = 0;
int out = 0;

// Semaforos conforme a la Figura 5.13 de Stallings
sem_t s; // Exclusor mutuo (inicializado en 1)
sem_t n; // Contador de items en el bufer (inicializado en 0)
sem_t e; // Contador de espacios vacios (inicializado en CAPACITY)

/*
 * Funcion ejecutada por cada hilo Productor
 */
void* producer(void* arg) {
    int id = *(int*)arg;

    for (int i = 0; i < ITEMS_PER_PROD; i++) {
        // Simulacion de tiempo de produccion
        usleep((rand() % 100 + 50) * 1000);

        // Imprimir mensaje de produccion segun el formato requerido
        printf("[P%d] Producing %d ...\n", id, i);
        fflush(stdout);

        // Esperar espacio libre en el bufer (decrementa e)
        sem_wait(&e);
        // Garantizar exclusion mutua para manipular el bufer
        sem_wait(&s);

        // Seccion Critica: Insercion en el bufer circular
        buffer[in] = i;
        in = (in + 1) % CAPACITY;

        // Liberar exclusion mutua
        sem_post(&s);
        // Incrementar contador de items disponibles (incrementa n)
        sem_post(&n);
    }

    pthread_exit(NULL);
}

/*
 * Funcion ejecutada por cada hilo Consumidor
 */
void* consumer(void* arg) {
    int id = *(int*)arg;

    while (1) {
        // Esperar item disponible en el bufer (decrementa n)
        sem_wait(&n);
        // Garantizar exclusion mutua para manipular el bufer
        sem_wait(&s);

        // Seccion Critica: Extraccion del bufer circular
        int item = buffer[out];
        out = (out + 1) % CAPACITY;

        // Liberar exclusion mutua
        sem_post(&s);
        // Incrementar contador de espacios libres (incrementa e)
        sem_post(&e);

        // Imprimir mensaje de consumo segun el formato requerido
        printf("-----> [C%d] consumed %d\n", id, item);
        fflush(stdout);

        // Simulacion de tiempo de consumo
        usleep((rand() % 150 + 50) * 1000);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t prods[NUM_PRODUCERS];
    pthread_t cons[NUM_CONSUMERS];
    int prod_ids[NUM_PRODUCERS];
    int cons_ids[NUM_CONSUMERS];

    srand(time(NULL));

    // Inicializacion de semaforos segun la Figura 5.13 de Stallings
    sem_init(&s, 0, 1);        // s = 1 (exclusion mutua)
    sem_init(&n, 0, 0);        // n = 0 (items producidos)
    sem_init(&e, 0, CAPACITY); // e = CAPACITY (espacios vacios)

    // Creacion de hilos productores (P0, P1, P2)
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        prod_ids[i] = i;
        pthread_create(&prods[i], NULL, producer, &prod_ids[i]);
    }

    // Creacion de hilos consumidores (C0, C1, C2)
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        cons_ids[i] = i;
        pthread_create(&cons[i], NULL, consumer, &cons_ids[i]);
    }

    // Esperar a que todos los hilos productores terminen su trabajo
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(prods[i], NULL);
    }

    // Tiempo adicional para permitir el consumo de los ultimos elementos
    sleep(2);

    // Destruccion de semaforos
    sem_destroy(&s);
    sem_destroy(&n);
    sem_destroy(&e);

    return 0;
}