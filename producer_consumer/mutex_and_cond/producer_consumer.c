/******************************************************************************
 * File: producer_consumer.c
 * Author: Patrick Payne, Modified from code by Andrew S. Tanenbaum.
 * Date Created: Feb 25, 2014
 * Purpose: Snippet demonstrating a basic approach for dealing with the
 *  producer-consumer problem in multithreaded applications. Basic code taken
 *  from "Modern Operating Systems 3e" figure 232, page 135.
 *****************************************************************************/
#include <stdio.h>
#include <pthread.h>
#define MAX 100
pthread_mutex_t the_mutex;
pthread_cond_t condc, condp;
int buffer = 0;

void *producer(void *ptr) {
  for (int i = 1; i <= MAX; i++) {
    pthread_mutex_lock(&the_mutex);
    while (buffer != 0) {
      pthread_cond_wait(&condp, &the_mutex);
    }

    buffer = i;
    printf("Producing %d\n", buffer);
    pthread_cond_signal(&condc);
    pthread_mutex_unlock(&the_mutex);
  }
  pthread_exit(0);
}

void *consumer(void *ptr) {
  for (int i = 1; i <= MAX; i++) {
    pthread_mutex_lock(&the_mutex);
    while(buffer == 0) pthread_cond_wait(&condc, &the_mutex);
    printf("Consuming %d\n", buffer);
    buffer = 0;
    pthread_cond_signal(&condp);
    pthread_mutex_unlock(&the_mutex);
  }
  pthread_exit(0);
}

int main(void) {
  pthread_t pro, con;
  pthread_mutex_init(&the_mutex, 0);
  pthread_cond_init(&condc, 0);
  pthread_cond_init(&condp, 0);
  pthread_create(&con, 0, consumer, 0);
  pthread_create(&pro, 0, producer, 0);
  pthread_join(pro, 0);
  pthread_join(con, 0);
  pthread_cond_destroy(&condc);
  pthread_cond_destroy(&condp);
  pthread_mutex_destroy(&the_mutex);
}
