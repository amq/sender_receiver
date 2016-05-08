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

static long parse_shm_size(int argc, char *argv[]);
static int init(long shm_size);
static int cleanup(void);
static void signal_callback(int signum);
static int write_to_shm(long shm_size, FILE *stream);
static int read_from_shm(long shm_size);

#endif /* _SENDER_RECEIVER_H_ */
