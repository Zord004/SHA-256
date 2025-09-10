#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <openssl/sha.h>

void digest_file(const char *filename, uint8_t * hash) {

    SHA256_CTX ctx;
    SHA256_Init(&ctx);

    char buffer[32];

    int file = open(filename, O_RDONLY, 0);
    if (file == -1) {
        printf("File %s non esiste \n", filename);
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_read;
    do {
        // read the file in chunks of 32 characters
        bytes_read = read(file, buffer, 32);
        if (bytes_read > 0) {
            SHA256_Update(&ctx,(uint8_t *)buffer,bytes_read);
        } else if (bytes_read < 0) {
            printf("Lettura fallita\n");
            exit(EXIT_FAILURE);
        }
    } while (bytes_read > 0);

    SHA256_Final(hash, &ctx);

    close(file);
}


void hash_file(char *file_name, char* buf) {
  uint8_t hash[32];
  digest_file(file_name, hash);

  for (int i = 0; i < 32; i++)
    sprintf(buf + (i * 2), "%02x", hash[i]);

  buf[64] = '\0';
}
