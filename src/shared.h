#ifndef _SENDER_RECEIVER_H_
#define _SENDER_RECEIVER_H_

#include <stdio.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define NAME_LENGTH 16
/* names expect an unsigned int */
#define SEM_R_NAME 1000 * getuid() + 0
#define SEM_W_NAME 1000 * getuid() + 1
#define SHM_NAME 1000 * getuid() + 2

long parse_shm_size(int argc, char *argv[]);
int write_to_shm(long shm_size, FILE *stream);
int read_from_shm(long shm_size);

#endif /* _SENDER_RECEIVER_H_ */
