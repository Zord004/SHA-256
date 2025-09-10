#include <pthread.h>
#include <stdbool.h>
#define CACHE_SIZE 100
#define PATH_LEN 100
#define HASH_LEN 65

typedef struct {
  char path[PATH_LEN];
  char hash[HASH_LEN];
} cache_entry;

typedef struct {
  cache_entry entries[CACHE_SIZE];
  int count;
  pthread_mutex_t mtx;
} cache_t;

void cache_create(cache_t *c);
void cache_destroy(cache_t *c);
void cache_print(cache_t *c);
void cache_insert(cache_t *c, const char *path, const char *hash);
bool cache_get(cache_t *c, const char *path, char *hash_buf);
