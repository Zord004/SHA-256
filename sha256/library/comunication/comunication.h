#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#define PATH_LEN 100
#define HASH_LEN 65

extern volatile sig_atomic_t stop_flag;

typedef struct {
  char path[PATH_LEN];
  char response_fifo[PATH_LEN];
  int file_size;
  bool is_valid;
} request;

typedef struct {
  char path[PATH_LEN];
  char hash[HASH_LEN];
} response;

typedef struct queue_node {
  request req;
  struct queue_node *next;
} node_t;

typedef struct {
  node_t *head;
  pthread_mutex_t mtx;
  pthread_cond_t cond;
} queue_t;

void err_exit(const char *msg);

void queue_create(queue_t *q);
void queue_print(queue_t *q);
bool queue_is_empty(queue_t *q);
bool queue_is_full(queue_t *q);
void queue_push(queue_t *q, request req);
request queue_pop(queue_t *q);
void queue_destroy(queue_t *q);
void queue_stop(queue_t *q);
