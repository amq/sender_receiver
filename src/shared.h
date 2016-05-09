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
#include <err.h>

#define IPC_NAME_LEN 24
#define SEM_W_NAME (1000ull * getuid() + 0)
#define SEM_R_NAME (1000ull * getuid() + 1)
#define SHM_NAME (1000ull * getuid() + 2)

long shared_parse_size(int argc, char *argv[]);
int shared_init(long shm_size);
void shared_cleanup(void);
void shared_signal(int signum);
int shared_send(long shm_size, FILE *stream);
int shared_receive(long shm_size);

#endif /* _SENDER_RECEIVER_H_ */
