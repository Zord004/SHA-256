#include "pending_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void pending_list_create(pending_list_t *pl) {
  pthread_mutex_init(&pl->mtx, NULL);
  pthread_cond_init(&pl->cond, NULL);
  pl->head = NULL;
}

void pending_list_destroy(pending_list_t *pl) {
  pthread_mutex_lock(&pl->mtx);

  pending_node_t *current = pl->head;
  while (current != NULL) {
    pending_node_t *temp = current;
    current = current->next;
    free(temp);
  }

  pthread_mutex_unlock(&pl->mtx);
  pthread_mutex_destroy(&pl->mtx);
  pthread_cond_destroy(&pl->cond);
}

bool pending_list_contains(pending_list_t *pl, const char *path) {
  pthread_mutex_lock(&pl->mtx);

  pending_node_t *current = pl->head;
  while (current != NULL) {
    if (strcmp(current->path, path) == 0) {
      pthread_mutex_unlock(&pl->mtx);
      return true;
    }
    current = current->next;
  }

  pthread_mutex_unlock(&pl->mtx);
  return false;
}

bool pending_list_contains_nolock(pending_list_t *pl, const char *path) {
  pending_node_t *current = pl->head;
  while (current != NULL) {
    if (strcmp(current->path, path) == 0) {
      pthread_mutex_unlock(&pl->mtx);
      return true;
    }
    current = current->next;
  }
  return false;
}

void pending_list_add(pending_list_t *pl, const char *path) {
  pthread_mutex_lock(&pl->mtx);

  pending_node_t *current = pl->head;
  while (current != NULL) {
    if (strcmp(current->path, path) == 0) {
      pthread_mutex_unlock(&pl->mtx);
      return;
    }
    current = current->next;
  }

  pending_node_t *new_node = malloc(sizeof(pending_node_t));
  strncpy(new_node->path, path, PATH_LEN);
  new_node->next = pl->head;
  pl->head = new_node;

  pthread_mutex_unlock(&pl->mtx);
}

void pending_list_remove(pending_list_t *pl, const char *path) {
  pthread_mutex_lock(&pl->mtx);

  pending_node_t **current = &pl->head;
  while (*current != NULL) {
    pending_node_t *entry = *current;
    if (strcmp(entry->path, path) == 0) {
      *current = entry->next;
      free(entry);
      pthread_cond_broadcast(&pl->cond);
      pthread_mutex_unlock(&pl->mtx);
      return;
    }
    current = &entry->next;
  }

  pthread_mutex_unlock(&pl->mtx);
}

void pending_list_wait_until_removed(pending_list_t *pl, const char *path) {
  pthread_mutex_lock(&pl->mtx);

  while (pending_list_contains_nolock(pl, path) && !stop_flag) {
    pthread_cond_wait(&pl->cond, &pl->mtx);
  }

  if (stop_flag) {
    pthread_mutex_unlock(&pl->mtx);
    pthread_exit(NULL);
  }

  pthread_mutex_unlock(&pl->mtx);
}

void pending_list_stop(pending_list_t *pl) {
  pthread_mutex_lock(&pl->mtx);
  stop_flag = 1;
  pthread_cond_broadcast(&pl->cond);
  pthread_mutex_unlock(&pl->mtx);
}

void pending_list_print(pending_list_t *pl) {
  pthread_mutex_lock(&pl->mtx);

  if (pl->head == NULL) {
    printf("[]\n");
    pthread_mutex_unlock(&pl->mtx);
    return;
  }

  printf("[\n");
  pending_node_t *current = pl->head;
  while (current != NULL) {
    printf("  %s\n", current->path);
    current = current->next;
  }
  printf("]\n");

  pthread_mutex_unlock(&pl->mtx);
}
