#include <fcntl.h>
#include <openssl/sha.h>
#include <unistd.h>

#define HASH_LEN 65

static char *request_fifo = "/tmp/sha256_fifo_request";

void hash_file(char *file_name, char* buf);
