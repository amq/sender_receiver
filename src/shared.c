#include "shared.h"

/* write semaphore: free space */
char sem_w_name[NAME_LENGTH] = "";
sem_t *sem_w_id = SEM_FAILED;

/* read semaphore: number of characters left to be read */
char sem_r_name[NAME_LENGTH] = "";
sem_t *sem_r_id = SEM_FAILED;

char shm_name[NAME_LENGTH] = "";
int shm_fd = -1;
int *shm_buffer = MAP_FAILED;

long size = -1;

/**
 * @brief
 *
 * @param
 *
 * @returns
 */
long parse_shm_size(int argc, char *argv[]) {
  int opt = -1;
  long shm_size = -1;
  char *notconv = "";

  /* there are no arguments */
  if (argc < 2) {
    errno = EINVAL;
    return -1;
  }

  while ((opt = getopt(argc, argv, "m:")) != -1) {
    switch (opt) {
    case 'm':
      errno = 0;
      shm_size = strtol(optarg, &notconv, 10);
      /* over-/underflow or some characters could not be converted */
      if (errno != 0 || *notconv != '\0') {
        errno = EINVAL;
        return -1;
      }
      break;
    default:
      errno = EINVAL;
      return -1;
    }
  }

  /* there are some non-option arguments */
  if (optind < argc) {
    errno = EINVAL;
    return -1;
  }

  return shm_size;
}

/**
 * @brief
 *
 * it is safe to call this function multiple times,
 * sem_open and shm_open only create elements if they don't exist
 *
 * @param
 *
 * @returns
 */
int init(long shm_size) {
  size = shm_size;

  /* @todo: portability */
  signal(SIGINT, signal_callback);
  signal(SIGTERM, signal_callback);
  signal(SIGHUP, signal_callback);

  /* convert the sem/shm number (as defined in spec) to a string */
  if (sprintf(sem_w_name, "%llu", SEM_W_NAME) < 0) {
    /* errno is set by sprintf */
    return -1;
  }
  if (sprintf(sem_r_name, "%llu", SEM_R_NAME) < 0) {
    return -1;
  }
  if (sprintf(shm_name, "%llu", SHM_NAME) < 0) {
    return -1;
  }

  /* initially the free space is equal to shm_size */
  sem_w_id = sem_open(sem_w_name, O_CREAT, S_IRUSR | S_IWUSR, shm_size);

  /* initially there are no characters to be read */
  sem_r_id = sem_open(sem_r_name, O_CREAT, S_IRUSR | S_IWUSR, 0);

  if (sem_w_id == SEM_FAILED || sem_r_id == SEM_FAILED) {
    /* errno is set by sem_open */
    return -1;
  }

  /* @todo: make sure we understand the options */
  /* open a shared memory object */
  shm_fd = shm_open(shm_name, O_RDWR | O_CREAT,
                    S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH);

  if (shm_fd == -1) {
    /* errno is set by shm_open */
    return -1;
  }

  /* set the size of the shared memory object */
  if (ftruncate(shm_fd, shm_size * sizeof(*shm_buffer)) == -1) {
    /* errno is set by ftruncate */
    return -1;
  }

  /* map the memory object so that it can be used, imagine it as malloc */
  shm_buffer = mmap(NULL, (size_t)shm_size * sizeof(*shm_buffer), PROT_READ | PROT_WRITE,
                    MAP_SHARED, shm_fd, 0);

  if (shm_buffer == MAP_FAILED) {
    /* errno is set by mmap */
    return -1;
  }

  return 0;
}

/**
 * @brief
 *
 * @param
 *
 * @returns
 */
int cleanup(void) {
  int errno_save = errno;

  if (sem_w_id != SEM_FAILED) {
    sem_close(sem_w_id);
    sem_unlink(sem_w_name);
  }

  if (sem_r_id != SEM_FAILED) {
    sem_close(sem_r_id);
    sem_unlink(sem_r_name);
  }

  if (shm_buffer != MAP_FAILED) {
    munmap(shm_buffer, (size_t)size);
  }

  if (shm_fd != -1) {
    close(shm_fd);
    shm_unlink(shm_name);
  }

  /* make sure this function does not modify errno */
  errno = errno_save;

  fprintf(stderr, "cleanup\n");

  return 0;
}

void signal_callback(int signum) {
  cleanup();

  fprintf(stderr, "caught signal\n");

  exit(signum);
}

/**
 * @brief
 *
 * @param
 *
 * @returns
 */
int write_to_shm(long shm_size, FILE *stream) {
  int input = EOF;
  int position = 0;

  if (init(shm_size) == -1) {
    cleanup();
    /* errno is set by init */
    return -1;
  }

  do {
    /* decrement the free space and wait if 0 */
    while (sem_wait(sem_w_id) == -1) {
      /* try again after a signal interrupt */
      if (errno == EINTR) {
        continue;
      } else {
        cleanup();
        /* errno is set by sem_wait */
        return -1;
      }
    }

    input = fgetc(stream);
    shm_buffer[position] = input;

    printf("shm_buffer[%d] = %d\n", position, shm_buffer[position]);

    position++;

    /* wrap around the ring buffer */
    if (position == shm_size) {
      position = 0;
    }

    /* increment the number of characters */
    if (sem_post(sem_r_id) == -1) {
      cleanup();
      /* errno is set by sem_post */
      return -1;
    }
  } while (input != EOF);

  return 0;
}

/**
 * @brief
 *
 * @param
 *
 * @returns
 */
int read_from_shm(long shm_size) {
  int output = EOF;
  int position = 0;

  if (init(shm_size) == -1) {
    cleanup();
    /* errno is set by init */
    return -1;
  }

  do {
    /* decrement the number of characters and wait if 0 */
    while (sem_wait(sem_r_id) == -1) {
      /* try again after a signal interrupt */
      if (errno == EINTR) {
        continue;
      } else {
        cleanup();
        /* errno is set by sem_wait */
        return -1;
      }
    }

    output = shm_buffer[position];

    printf("\nshm_buffer[%d] = %d\n", position, output);

    if (output != EOF) {
      if (printf("%c", output) < 0) {
        cleanup();
        /* errno is set by printf */
        return -1;
      }
    }

    position++;

    /* wrap around the ring buffer */
    if (position == shm_size) {
      position = 0;
    }

    /* increment the free space */
    if (sem_post(sem_w_id) == -1) {
      cleanup();
      /* errno is set by sem_post */
      return -1;
    }
  } while (output != EOF);

  cleanup();

  return 0;
}
