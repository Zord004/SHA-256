#include "cache.h"
#include "comunication.h"
#include "pending_list.h"
#include "sha256_file.h"
#include <fcntl.h>
#include <openssl/sha.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_THREADS 5

queue_t queue;
cache_t cache;
pending_list_t pending_list;
pthread_mutex_t fifo_mtx = PTHREAD_MUTEX_INITIALIZER;

void leggiRisposta(int requests_fd);

void creaCanale();

void stop_server();

void inviaRisposta(int response_fd, response *res);

int is_directory(const char *path);

long get_file_size(const char *filename);

request verify_request(request req); 

void *add_requests(void *null);

void *gestisciRichieste(void *null);

int main(int argc, char *argv[]) {
  signal(SIGINT, (__sighandler_t)stop_server);

  queue_create(&queue);
  cache_create(&cache);
  pending_list_create(&pending_list);
  
  creaCanale();

  pthread_t requests_thread;
  if (pthread_create(&requests_thread, NULL, add_requests, NULL) == -1)
    err_exit("Errore nella creazione del thread");

  
  pthread_t thread_pool[MAX_THREADS]; //Iniziazlizzazione thread pool
  int first_free = 0;
  for (int i = 0; i < MAX_THREADS; i++) {
    if (pthread_create(&thread_pool[i], NULL, gestisciRichieste, NULL) == -1)
      err_exit("Errore di creazione del thread pool");
  }

  for (int i = 0; i < MAX_THREADS; i++) {
    pthread_join(thread_pool[i], NULL);
  }

  pthread_join(requests_thread, NULL);
}

void stop_server() {
  printf("\nStop server\n");

  queue_stop(&queue);
  pending_list_stop(&pending_list);
  pthread_mutex_destroy(&fifo_mtx);

  queue_destroy(&queue);
  cache_destroy(&cache);

  pending_list_destroy(&pending_list);

  if (unlink(request_fifo) == -1)
    err_exit("Errore FIFO");

  exit(EXIT_SUCCESS);
}

void creaCanale() {
  printf("Crea FIFO\n");
  if (mkfifo(request_fifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
    err_exit("Errore di creazione nella FIFO");

  printf("FIFO creata: %s\n", request_fifo);
}

request verify_request(request req) {
  bool file_exists = access(req.path, F_OK) == 0;
  if (!file_exists || is_directory(req.path)) {
    req.is_valid = false;
  } else {
    req.file_size = get_file_size(req.path);
    req.is_valid = true;
  }

  return req;
}

long get_file_size(const char *filename) {
  FILE *file = fopen(filename, "rb"); 
  if (file == NULL) {
    perror("Error apertura file");
    return -1;
  }

  if (fseek(file, 0, SEEK_END) != 0) {
    perror("Error file");
    fclose(file);
    return -1;
  }

  long size = ftell(file);
  if (size == -1L) {
    perror("Error dimensione file");
    fclose(file);
    return -1;
  }

  fclose(file);
  return size;
}

void leggiRisposta(int request_fd) {
  request client_req = {.path = {0}, .response_fifo = {0}};
  int bytes_read = 0;

  while ((bytes_read = read(request_fd, client_req.path, PATH_LEN)) ==
         PATH_LEN) {

    if (read(request_fd, client_req.response_fifo, PATH_LEN) < 0) {
      fprintf(stderr, "Errore di lettura del percorso della FIFO\n");
      exit(EXIT_FAILURE);
    }

    queue_push(&queue, verify_request(client_req));
  }

  if (bytes_read == -1)
    err_exit("Errore nella lettura da request FIFO");
}

void inviaRisposta(int response_fd, response *res) {
  pthread_mutex_lock(&fifo_mtx);

  if (write(response_fd, res, sizeof(response)) < sizeof(response))
    err_exit("Errore di apertura della risposta");

  pthread_mutex_unlock(&fifo_mtx);
}

int is_directory(const char *path) {
  struct stat statbuf;
  if (stat(path, &statbuf) != 0)
    return 0;
  return S_ISDIR(statbuf.st_mode);
}

void *add_requests(void *null) {
  printf("Attesa...\n");
  while (1) {
    int request_fd = open(request_fifo, O_RDONLY);
    if (request_fd == -1)
      err_exit("Errore apertura request FIFO");

    leggiRisposta(request_fd);

    if (close(request_fd) == -1)
      err_exit("Errore chiusura request FIFO");
  }
}

void *gestisciRichieste(void *null) {
  while (1) {
    request req = queue_pop(&queue);

    response res = {.path = {0}, .hash = {0}};
    if (strncpy(res.path, req.path, PATH_LEN) == NULL) {
      err_exit("Errore nel copiare il percorso");
    }

    int response_fd = open(req.response_fifo, O_WRONLY);
    if (response_fd == -1) {
      err_exit("Errore apertura response FIFO");
      continue;
    }

    if (!req.is_valid) {
      printf("%s: File non esiste\n", req.path);

      strncpy(res.hash, "File non esiste", HASH_LEN);
      inviaRisposta(response_fd, &res);

      if (close(response_fd) == -1)
        err_exit("Errore chiusura response FIFO");
      continue;
    }

    bool cache_hit = cache_get(&cache, req.path, res.hash);

    if (!cache_hit) {
      if (pending_list_contains(&pending_list, req.path)) {
        pending_list_wait_until_removed(&pending_list, req.path);

        cache_hit = cache_get(&cache, req.path, res.hash);
      }

      if (!cache_hit) {
        pending_list_add(&pending_list, req.path);

        hash_file(req.path, res.hash);
        cache_insert(&cache, req.path, res.hash);

        pending_list_remove(&pending_list, req.path);
      }
    }

    printf("%s: %s\n", req.path, res.hash);

    if (access(req.response_fifo, F_OK) == 0)
      inviaRisposta(response_fd, &res);

    if (close(response_fd) == -1)
      err_exit("Errore chiusura respone FIFO");
  }
}