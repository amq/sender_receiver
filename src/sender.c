#include "shared.h"

int main(int argc, char *argv[]) {
  long shm_size = shared_parse_size(argc, argv);

  if (shm_size == -1) {
    fprintf(stderr, "Usage: ./sender -m <buffer_size> < data.txt\n");
    return EXIT_FAILURE;
  }

  if (shared_send(shm_size, stdin) == -1) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
