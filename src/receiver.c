#include "shared.h"

/**
 * @brief prints out contents of the shared memory
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
    fprintf(stderr, "Usage: %s -m <buffer_size>\n", argv[0]);
    return EXIT_FAILURE;
  }

  if (shared_receive(shm_size, stdout) == -1) {
    /* error is printed by shared_receive() */
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
