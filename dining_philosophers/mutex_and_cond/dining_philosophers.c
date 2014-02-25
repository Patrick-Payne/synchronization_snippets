/******************************************************************************
 * File: dining_philosophers.c
 * Author: Patrick Payne
 * Date Created: Feb 25, 2014
 * Purpose: A solution to the dining philosophers problem using Posix mutexes
 *  and condition variables. This was inspired by the code in figure 2-46 in
 *  "Modern Operating Systems 3e", which gives an outline of the solution using
 *  abstracted semaphores. This implementation does not work with 3
 *  philosophers, but works with any other number.
 * Copyright 2014 by Patrick Payne.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>

/******************************************************************************
 * CONSTANTS AND MACROS
 *****************************************************************************/ 
#define N 5 /* The number of philosophers. The classic problem uses 5. */
#define MAX_EAT 5 /* The maximum number of times each philosopher eats. */
#define LEFT(i) ((i) + N - 1) % N /* Get the index of the left philosopher */
#define RIGHT(i) ((i) + 1) % N /* Get the index of the left philosopher */


/******************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/ 

/* The state of each philosopher is represented as the following enum. */
enum state_enum {THINKING, HUNGRY, EATING} state[N] = {};
pthread_mutex_t mutex; /* Mutex used by all threads before critical region. */

/* Threads wait on these condition variables when they are hungry but cannot
 * obtain the forks.
 */
pthread_cond_t waiting_to_eat[N];


/******************************************************************************
 * FUNCTION DEFINITIONS
 *****************************************************************************/ 

void eat(int phil_num, int eat_count) {
  printf("Philosopher %d ate serving #%d.\n", phil_num, eat_count);
}


void think(int phil_num) {
  printf("Philosopher %d is thinking...\n", phil_num);
}


bool can_eat(int phil_num) {
  return ((state[phil_num] == HUNGRY) &&
          (state[LEFT(phil_num)] != EATING) &&
          (state[RIGHT(phil_num)] != EATING));
}


void take_forks(int phil_num) {
  pthread_mutex_lock(&mutex);

  /* Go hungry and wait for the forks if they are not available. */
  state[phil_num] = HUNGRY;
  if(!can_eat(phil_num)) {
    pthread_cond_wait(&waiting_to_eat[phil_num], &mutex);
  }

  /* At this point, this philosopher can start eating. */
  state[phil_num] = EATING;

  pthread_mutex_unlock(&mutex);
}


void put_forks(int phil_num) {
  pthread_mutex_lock(&mutex);
  
  /* Stop eating and start thinking again. */
  state[phil_num] = THINKING;

  /* Now wake up neighbours that can now access forks. */
  if (can_eat(LEFT(phil_num))) {
    pthread_cond_signal(&waiting_to_eat[LEFT(phil_num)]);
  }
  if (can_eat(RIGHT(phil_num))) {
    pthread_cond_signal(&waiting_to_eat[RIGHT(phil_num)]);
  }

  pthread_mutex_unlock(&mutex);
}


void *philosopher(void *ptr) {
  /* Introduce philosopher upon entering restaurant. */
  int phil_number = *((int *) ptr);
  free(ptr);

  printf("Hi, I'm philosopher #%d.\n", phil_number);

  /* Keep thinking and eating alternately until we eat enough times. */
  int num_eats = 0;
  while (num_eats < MAX_EAT) {
    think(phil_number);
    take_forks(phil_number);
    num_eats++;
    eat(phil_number, num_eats);
    put_forks(phil_number);
  }

  pthread_exit(EXIT_SUCCESS);
}


/******************************************************************************
 * MAIN
 *****************************************************************************/ 
int main(void) {
  pthread_t philosopher_threads[N];

  /* Initialize all the condition variables. */
  pthread_mutex_init(&mutex, NULL);
  for (int i = 0; i < N; i++) {
    pthread_cond_init(&waiting_to_eat[i], NULL);
  }

  for (int i = 0; i < N; i++) {
    int *arg = malloc(sizeof(int));
    if (arg == NULL) {
      fprintf(stderr, "Error: %s.\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    *arg = i;
    pthread_create(&philosopher_threads[i], NULL, philosopher, (void *)arg);
  }

  /* Wait for all of the philosophers to eat their meals. */
  for (int i = 0; i < N; i++) {
    pthread_join(philosopher_threads[i], NULL);
  }

  /* Clear all synchronization primitives. */
  pthread_mutex_destroy(&mutex);
  for (int i = 0; i < N; i++) {
    pthread_cond_destroy(&waiting_to_eat[i]);
  }

  exit(EXIT_SUCCESS);
}
