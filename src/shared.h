#ifndef _SENDER_RECEIVER_H_
#define _SENDER_RECEIVER_H_

#include <stdio.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define NAME_LENGTH 24
#define SEM_W_NAME (1000ull * getuid() + 0)
#define SEM_R_NAME (1000ull * getuid() + 1)
#define SHM_NAME (1000ull * getuid() + 2)

long parse_shm_size(int argc, char *argv[]);
int init(long shm_size);
int cleanup(void);
void signal_callback(int signum);
int write_to_shm(long shm_size, FILE *stream);
int read_from_shm(long shm_size);

#endif /* _SENDER_RECEIVER_H_ */
