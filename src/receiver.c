#include "shared.h"

int main(int argc, char *argv[]) {
  long shm_size = parse_shm_size(argc, argv);

  /*
   * do not try running it yet
   *
   * if (read_from_shm(shm_size) == -1) {
   *   perror(argv[0]);
   *   return -1;
   * }
   */

  return 0;
}
