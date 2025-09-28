/* Code to simulate sleep barber problem
  Run: gcc sleep_barber.c -o exec -lpthread
*/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>

#define TRUE 1

/* Number of chairs for costurmers wait */
#define CHAIRS 5

/* Show if the barber is waiting */
sem_t barbers; 
/* Number of costumers waiting */
sem_t costumers;
/* Protect waiting (critic zone) */
pthread_mutex_t mutex;

int waiting = 0;

void *barber(void *arg);
void *costumer(void *id);
void cut_hair(void);

int main(void)
{
  pthread_t tid_barber;
  /* Have more costumers than chairs */
  pthread_t tid_costumers[CHAIRS + 5];
  int i;
  int ids_costumers[CHAIRS + 5];

  /* int sem_init(sem_t *sem, int pshared, unsigned int value) */
  // pshared -> 0 shared between threads from same process
  sem_init(&barbers, 0, 0);
  sem_init(&costumers, 0, 0);

  /* unlock mutex to be use */
  pthread_mutex_init(&mutex, NULL);

  printf("Start barber shop with %d waiting chairs.\n", CHAIRS);

  // Create barber thread -> program is divided in main and barber
  /* save id of thread, default thread, main da thread, arg func -> NULL */
  pthread_create(&tid_barber, NULL, barber, NULL);

  // Create costumers threads with gaps
  for(i = 0; i < CHAIRS + 5; i++)
  {
    ids_costumers[i] = i + 1;
    // ids_costumers[i] -> arg. function to costumer.
    pthread_create(&tid_costumers[i], NULL, costumer, &ids_costumers[i]);
    sleep(1); /* gap between costumers is 1 seg*/
  }

  // Waits end of costumers thread 
  for(int i = 0; i < CHAIRS + 5; i++)
  {
    // Sleep main thread waiting for tid_costumers[i].
    pthread_join(tid_costumers[i], NULL);
  }

  return 0;
}

void cut_hair(void)
{
  printf("Barber is cutting the hair... \n");
  sleep(3);
}

void *barber(void *arg)
{
  while(TRUE)
  {
    /* Need costumers to work, otherwise sleeps */
    printf("Barber is sleeping.\n");
    /* sem_wait decrease by 1 or block process(costumers = 0) */
    sem_wait(&costumers);

    // If costumer arrive, barber wakeup.
    /* enter critical zone - modify nº costumers waiting */
    pthread_mutex_lock(&mutex);

    /* Costumers ready to cut hair */
    waiting = waiting - 1;
    printf("Barber woke up. Occupied waiting chairs : %d\n", waiting);

    pthread_mutex_unlock(&mutex);

    /* Barber is ready to cut the hair */
    sem_post(&barbers);

    cut_hair();
    printf("The barber finished the haircut.\n");    
  }
}

void *costumer(void *id)
{
  int costumer_id = *(int *)id;
  printf("Costumer %d has arrived.\n", costumer_id);

  /* waiting could change after if, so lock */
  pthread_mutex_lock(&mutex);
  
  if(waiting < CHAIRS)
  {
    // Has chair so costumer wait
    waiting = waiting + 1;
    printf("Costumer %d is waiting for a haircut. Has %d costumers waiting\n", 
          costumer_id, waiting);
    
    /* Increment costumers by 1, if any process is sleeping
    because a sem_wait call, so operacional system wake up
    one of them. Has to be the same semaphore. */
    sem_post(&costumers);

    /* sem_post come first to avoid lost a wakeup signal */
    pthread_mutex_unlock(&mutex);

    /* if the barber is cutting(0), costumers sleep */
    sem_wait(&barbers);

    printf("Costumer %d is getting a haircut.\n", costumer_id);
  }
  else
  {
    /* Dont have chair for the new costumer,
    so he will not wait for the haircut. */
    pthread_mutex_unlock(&mutex);
    printf("Full barbershop. Costumers %d leaves.\n", costumer_id);
  }

  // Ends thread and return NULL
  pthread_exit(NULL);
}
