#include "shared.h"

int main(int argc, char *argv[]) {
  long shm_size = parse_shm_size(argc, argv);

  if (shm_size == -1) {
    perror(argv[0]);
    return EXIT_FAILURE;
  }

  if (read_from_shm(shm_size) == -1) {
    perror(argv[0]);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
