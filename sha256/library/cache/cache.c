#include "cache.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>

void cache_create(cache_t *c) {
  pthread_mutex_init(&c->mtx, NULL);
  c->count = 0;
}

void cache_destroy(cache_t *c) {
  pthread_mutex_destroy(&c->mtx);
}

void cache_insert(cache_t *c, const char *path, const char *hash) {
  pthread_mutex_lock(&c->mtx);

  int insert_idx = c->count % CACHE_SIZE;

  strncpy(c->entries[c->count].path, path, PATH_LEN);
  strncpy(c->entries[c->count].hash, hash, HASH_LEN);
  c->count++;

  pthread_mutex_unlock(&c->mtx);
}

bool cache_get(cache_t *c, const char *path, char *hash_buf) {
  pthread_mutex_lock(&c->mtx);

  for (int i = 0; i < c->count; i++) {
    if (strcmp(c->entries[i].path, path) == 0) {
      strncpy(hash_buf, c->entries[i].hash, HASH_LEN);

      pthread_mutex_unlock(&c->mtx);
      return true;
    }
  }

  pthread_mutex_unlock(&c->mtx);
  return false;
}

void cache_print(cache_t *c) {
  pthread_mutex_lock(&c->mtx);

  if (c->count == 0) {
    printf("[]\n");
    return;
  }

  printf("[\n");
  for (int i = 0; i < c->count; i++) {
    printf("  (path: %s, hash: %s)", c->entries[i].path, c->entries[i].hash);
    if (i != c->count - 1) {
      printf(",");
    }
    printf("\n");
  }
  printf("]\n\n");

  pthread_mutex_unlock(&c->mtx);
}
