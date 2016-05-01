#include "shared.h"

key_t shm_key = 0;
key_t sem_r_key = 1;
key_t sem_w_key = 2;

/* dynamic key generation */
void generate_keys(void) {
    shm_key = 1000 * getuid() + shm_key;
    sem_r_key = 1000 * getuid() + sem_r_key;
    sem_w_key = 1000 * getuid() + sem_w_key;
}

void init(void) {
    generate_keys();
}