#include <pthread.h>
#include <signal.h>
#include <stdbool.h>

#define PATH_LEN 100

extern volatile sig_atomic_t stop_flag;

typedef struct pending_node {
  char path[PATH_LEN];
  struct pending_node *next;
} pending_node_t;

typedef struct pending_list {
  pthread_mutex_t mtx;
  pthread_cond_t cond;
  struct pending_node *head;
} pending_list_t;

void pending_list_create(pending_list_t *pl);
void pending_list_destroy(pending_list_t *pl);
bool pending_list_contains(pending_list_t *pl, const char *path);
void pending_list_add(pending_list_t *pl, const char *path);
void pending_list_remove(pending_list_t *pl, const char *path);
void pending_list_wait_until_removed(pending_list_t *pl, const char *path);
void pending_list_stop(pending_list_t *pl);
void pending_list_print(pending_list_t *pl);
