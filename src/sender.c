#include "shared.h"

/**
 * @brief records stdin into the shared memory
 *
 * @param argc the number of arguments
 * @param argv the arguments
 *
 * @returns EXIT_SUCCESS, EXIT_FAILURE
 */
int main(int argc, char *argv[]) {
  long shm_size;

  if ((shm_size = shared_parse_size(argc, argv)) == -1) {
    /* error is printed by shared_parse_size() */
    fprintf(stderr, "Usage: ./sender -m <buffer_size> < data\n");
    return EXIT_FAILURE;
  }

  if (shared_send(shm_size, stdin) == -1) {
    /* error is printed by shared_send() */
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
