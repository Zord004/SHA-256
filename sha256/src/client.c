#include "comunication.h"
#include "sha256_file.h"
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BASE_FIFO_PATH "/tmp/sha256_fifo_response"

void stop_client();

void inviaRichiesta(int argc, char *argv[], char *response_fifo_path); //Invia una richiesta alla request FIFO e la passa al server, maggiori dettagli nella relazione

char *creaRichiestaFIFO(char *base_path);

void leggiRichiesta(response *res, int response_fd); 

int main(int argc, char *argv[]) {
  signal(SIGINT, (__sighandler_t)stop_client);

  if (argc < 2) {
    fprintf(stderr, "%s [file_names]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  printf("Calcolando...\n");

  char *response_fifo_path = creaRichiestaFIFO(BASE_FIFO_PATH);
  if (mkfifo(response_fifo_path, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
    err_exit("Errore di creazione nella FIFO");

  inviaRichiesta(argc, argv, response_fifo_path);

  int response_fd = open(response_fifo_path, O_RDONLY);
  if (response_fd == -1)
    err_exit("Errore apertura FIFO");

  for (int i = 1; i < argc; i++) {
    response res;

     leggiRichiesta(&res, response_fd);

    printf("%s: %s\n", res.path, res.hash);
  }

  if (close(response_fd) == -1)
    err_exit("Errore chiusura FIFO");
  free(response_fifo_path);

  exit(EXIT_SUCCESS);
}

void inviaRichiesta(int argc, char *argv[], char *response_fifo_path) {
  int request_fd = open(request_fifo, O_WRONLY);
  if (request_fd == -1) {
    err_exit("Server non trovato");
  }

  for (int i = 1; i < argc; i++) {
    char *path = argv[i];

    if (write(request_fd, path, PATH_LEN) == -1)
      err_exit("Errore nella richiesta al server");

    if (write(request_fd, response_fifo_path, PATH_LEN) == -1)
      err_exit("Errore nella richiesta al server");
  }

  if (close(request_fd) == -1)
    err_exit("Errore chiusura FIFO");
}

void leggiRichiesta(response *res, int response_fd) {
  if (read(response_fd, res, sizeof(response)) < sizeof(response)) {
    fprintf(stderr, "Errore lettura richiesta\n");
    exit(EXIT_FAILURE);
  }
}

void stop_client() {
  printf("\nTerminando il client\n");
  unlink(creaRichiestaFIFO(BASE_FIFO_PATH));
  exit(EXIT_SUCCESS);
}

char *creaRichiestaFIFO(char *base_path) {
  int pid = getpid();
  char *response_fifo_path = malloc(PATH_LEN);

  if (response_fifo_path == NULL)
    err_exit("Errore nell'allocare la FIFO");

  sprintf(response_fifo_path, "%s_%d", base_path, pid);

  return response_fifo_path;
}