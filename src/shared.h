#pragma once

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
#define IPC_NAME(offset) (1000ull * getuid() + offset)
#define IPC_PERMS 0600

typedef struct {
    char shm_name[IPC_NAME_LEN];
    char sem_w_name[IPC_NAME_LEN];
    char sem_r_name[IPC_NAME_LEN];
    long shm_size;
    int shm_fd;
    int *shm_buffer;
    sem_t *sem_w_id;
    sem_t *sem_r_id;
} shared_t;

long shared_parse_size(int argc, char *argv[]);
int shared_send(long shm_size, FILE *stream);
int shared_receive(long shm_size);
