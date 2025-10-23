/*
* This program demonstrates the producer-consumer problem using pthreads.
* It creates a number of producer threads and consumer threads that
* share a fixed-size buffer. Semaphores and a mutex are used for synchronization.
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>

#define BUFFER_SIZE 10   // Reduced for clearer output
#define N_producers 5    // Reduced for clearer output
#define M_consumers 5    // Reduced for clearer output

typedef struct buffer_
{
    /* Current number of items in the buffer, also the next position to insert */
    int num_items;
    /* The shared buffer array */
    int item[BUFFER_SIZE];
} BUFFER;

/* The shared buffer */
BUFFER buffer = { 0, {0} };

/* Semaphores and Mutex */
sem_t empty;           // Counts empty slots in the buffer
sem_t full;            // Counts filled slots in the buffer
pthread_mutex_t mutex; // Ensures mutual exclusion for buffer access

/* Function prototypes */
void *producer(void *id);
void *consumer(void *id);

int main(void)
{
    pthread_t tid_producer[N_producers];
    pthread_t tid_consumer[M_consumers];
    int i;
    int ids_producer[N_producers];
    int ids_consumer[M_consumers];

    /* 1. Initialize the semaphores and mutex */
    // 'empty' starts at BUFFER_SIZE because all slots are initially empty.
    sem_init(&empty, 0, BUFFER_SIZE);
    // 'full' starts at 0 because no slots are filled yet.
    sem_init(&full, 0, 0);
    // Initialize the mutex for controlling access to the critical section.
    pthread_mutex_init(&mutex, NULL);

    /* 2. Create producer threads */
    for(i = 0; i < N_producers; i++)
    {
        ids_producer[i] = i + 1; // Assign a unique ID (1, 2, 3...)
        if(pthread_create(&tid_producer[i], NULL, producer, &ids_producer[i]) != 0)
        {
            perror("Failed to create producer thread");
        }
    }

    /* 3. Create consumer threads */
    for(i = 0; i < M_consumers; i++)
    {
        ids_consumer[i] = i + 1; // Assign a unique ID (1, 2, 3...)
        if(pthread_create(&tid_consumer[i], NULL, consumer, &ids_consumer[i]) != 0)
        {
            perror("Failed to create consumer thread");
        }
    }

    /* 4. Wait for all producer threads to finish */
    for(i = 0; i < N_producers; i++)
    {
        if(pthread_join(tid_producer[i], NULL) != 0)
        {
            perror("Failed to join producer thread");
        }
    }

    /* 5. Wait for all consumer threads to finish */
    for(i = 0; i < M_consumers; i++)
    {
        if(pthread_join(tid_consumer[i], NULL) != 0)
        {
            perror("Failed to join consumer thread");
        }
    }
    
    printf("All threads finished. Final buffer count: %d\n", buffer.num_items);

    /* 6. Clean up: destroy semaphores and mutex */
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    return 0;
}

/**
 * @brief The producer thread function.
 * Produces one item and places it in the buffer.
 */
void *producer(void *id)
{
    int producer_id = *(int *) id;
    int item = producer_id * 100; // Create a unique item to produce

    // Wait for an empty slot to become available.
    sem_wait(&empty);
    
    // Lock the mutex to enter the critical section.
    pthread_mutex_lock(&mutex);

    /* --- CRITICAL SECTION START --- */
    buffer.item[buffer.num_items] = item;
    buffer.num_items++;
    printf("Producer %d produced item %d. Buffer size is now %d\n", producer_id, item, buffer.num_items);
    /* --- CRITICAL SECTION END --- */

    // Unlock the mutex to exit the critical section.
    pthread_mutex_unlock(&mutex);
    
    // Signal that a slot is now full.
    sem_post(&full);

    pthread_exit(NULL);
}

/**
 * @brief The consumer thread function.
 * Consumes one item from the buffer.
 */
void *consumer(void *id)
{
    int consumer_id = *(int *) id;
    int item_consumed;

    // Wait for a filled slot to become available.
    sem_wait(&full);

    // Lock the mutex to enter the critical section.
    pthread_mutex_lock(&mutex);

    /* --- CRITICAL SECTION START --- */
    // Note: The buffer is treated like a stack (LIFO).
    buffer.num_items--;
    item_consumed = buffer.item[buffer.num_items];
    printf("Consumer %d consumed item %d. Buffer size is now %d\n", consumer_id, item_consumed, buffer.num_items);
    /* --- CRITICAL SECTION END --- */
    
    // Unlock the mutex to exit the critical section.
    pthread_mutex_unlock(&mutex);
    
    // Signal that a slot is now empty.
    sem_post(&empty);

    pthread_exit(NULL);
}
