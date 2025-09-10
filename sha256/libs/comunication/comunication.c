#include "comunication.h"
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

volatile sig_atomic_t stop_flag = 0;

void err_exit(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

void queue_create(queue_t *q) {
  pthread_mutex_init(&q->mtx, NULL);
  pthread_cond_init(&q->cond, NULL);

  q->head = NULL;
}

void queue_print(queue_t *q) {
  pthread_mutex_lock(&q->mtx);

  if (queue_is_empty(q)) {
    printf("[]\n");
    pthread_mutex_unlock(&q->mtx);
    return;
  }

  printf("[\n");
  node_t *current = q->head;
  while (current) {
    printf("  {\n    Percorso: %s\n    Dimensione file: %d\n    Valido: %d\n    "
           "response_fifo: %s\n  }",
           current->req.path, current->req.file_size, current->req.is_valid,
           current->req.response_fifo);
    if (current->next) {
      printf(",");
    }
    printf("\n");
    current = current->next;
  }
  printf("]\n\n");

  pthread_mutex_unlock(&q->mtx);
}

bool queue_is_empty(queue_t *q) { return q->head == NULL; }

void queue_push(queue_t *q, request req) {
  pthread_mutex_lock(&q->mtx);

  // Create new node
  node_t *new_node = malloc(sizeof(node_t));
  if (!new_node) {
    perror("malloc fallita");
    pthread_mutex_unlock(&q->mtx);
    exit(EXIT_FAILURE);
  }
  new_node->req = req;
  new_node->next = NULL;

  // Insert in ascending filesize order
  node_t **current = &q->head;
  while (*current && (*current)->req.file_size <= req.file_size) {
    current = &(*current)->next;
  }

  // Insert node
  new_node->next = *current;
  *current = new_node;

  pthread_mutex_unlock(&q->mtx);
  pthread_cond_signal(&q->cond);
}

request queue_pop(queue_t *q) {
  pthread_mutex_lock(&q->mtx);

  while (queue_is_empty(q) && !stop_flag) {
    pthread_cond_wait(&q->cond, &q->mtx);
  }

  if (stop_flag) {
    pthread_mutex_unlock(&q->mtx);
    pthread_exit(NULL);
  }

  // Remove from head
  node_t *temp = q->head;
  request res = temp->req;
  q->head = q->head->next;

  pthread_mutex_unlock(&q->mtx);

  free(temp);
  return res;
}

void queue_destroy(queue_t *q) {
  pthread_mutex_lock(&q->mtx);

  // Free all nodes in the queue
  node_t *current = q->head;
  while (current != NULL) {
    node_t *temp = current;
    current = current->next;
    free(temp);
  }
  q->head = NULL;

  pthread_mutex_unlock(&q->mtx);

  pthread_mutex_destroy(&q->mtx);
  pthread_cond_destroy(&q->cond);
}

void queue_stop(queue_t *q) {
  // Set the stop flag to signal the threads to exit
  pthread_mutex_lock(&q->mtx);
  stop_flag = 1;
  pthread_cond_broadcast(&q->cond);
  pthread_mutex_unlock(&q->mtx);
}
