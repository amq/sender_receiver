#include "shared.h"

/* write semaphore: free space */
static char sem_w_name[IPC_NAME_LEN] = "";
static sem_t *sem_w_id = SEM_FAILED;

/* read semaphore: number of characters left to be read */
static char sem_r_name[IPC_NAME_LEN] = "";
static sem_t *sem_r_id = SEM_FAILED;

/* shared memory */
static long global_shm_size = -1;
static char shm_name[IPC_NAME_LEN] = "";
static int shm_fd = -1;
static int *shm_buffer = MAP_FAILED;

/**
 * @brief parses the shared memory size from argv
 *
 * @param argc the number of arguments
 * @param argv the arguments
 *
 * @returns the shared memory size or -1 in case of error
 */
long shared_parse_size(int argc, char *argv[]) {
  int opt;
  long shm_size = -1;
  char *notconv = "";

  if (argc < 2) {
    warnx("Not enough arguments");
    return -1;
  }

  while ((opt = getopt(argc, argv, "m:")) != -1) {
    switch (opt) {
    case 'm':
      errno = 0;
      shm_size = strtol(optarg, &notconv, 10);
      /* over-/underflow or some characters could not be converted */
      if (errno != 0 || *notconv != '\0') {
        warnx("Invalid argument to: %c", opt);
        return -1;
      }
      break;
    default:
      /* error is printed by getopt() */
      return -1;
    }
  }

  if (optind < argc) {
    warnx("Non-option arguments");
    return -1;
  }

  return shm_size;
}

/**
 * @brief creates or opens semaphores and the shared memory
 *
 * @param shm_size the desired size of the shared memory
 *
 * @returns 0 if everything went well and -1 in case of error
 */
int shared_init(long shm_size) {
  global_shm_size = shm_size;

  /* register the signal handler */
  if (signal(SIGINT, shared_signal) == SIG_ERR) {
    warn("signal()");
    return -1;
  }
  if (signal(SIGTERM, shared_signal) == SIG_ERR) {
    warn("signal()");
    return -1;
  }
  if (signal(SIGHUP, shared_signal) == SIG_ERR) {
    warn("signal()");
    return -1;
  }

  /* convert the sem/shm number (as defined in spec) to a string */
  if (sprintf(sem_w_name, "%llu", SEM_W_NAME) < 0) {
    warn("sprintf()");
    return -1;
  }
  if (sprintf(sem_r_name, "%llu", SEM_R_NAME) < 0) {
    warn("sprintf()");
    return -1;
  }
  if (sprintf(shm_name, "%llu", SHM_NAME) < 0) {
    warn("sprintf()");
    return -1;
  }

  /* initially the free space is equal to shm_size */
  if ((sem_w_id = sem_open(sem_w_name, O_CREAT, 0600, shm_size)) == SEM_FAILED) {
    warn("sem_open()");
    return -1;
  }

  /* initially there are no characters to be read */
  if ((sem_r_id = sem_open(sem_r_name, O_CREAT, 0600, 0)) == SEM_FAILED) {
    warn("sem_open()");
    return -1;
  }

  /* open a shared memory object */
  if ((shm_fd = shm_open(shm_name, O_RDWR | O_CREAT, 0600)) == -1) {
    warn("shm_open()");
    return -1;
  }

  /* set the size of the shared memory object */
  if (ftruncate(shm_fd, shm_size * sizeof(*shm_buffer)) == -1) {
    warn("ftruncate()");
    return -1;
  }

  /* map the memory object so that it can be used, imagine it as malloc */
  if ((shm_buffer = mmap(NULL, (size_t)shm_size * sizeof(*shm_buffer), PROT_READ | PROT_WRITE,
                         MAP_SHARED, shm_fd, 0)) == MAP_FAILED) {
    warn("mmap()");
    return -1;
  }

  return 0;
}

/**
 * @brief closes and removes semaphores and the shared memory, best-effort
 */
void shared_cleanup(void) {
  if (sem_w_id != SEM_FAILED) {
    if (sem_close(sem_w_id) == -1) {
      warn("sem_close()");
    }
    if (sem_unlink(sem_w_name) == -1) {
      warn("sem_unlink()");
    }
  }

  if (sem_r_id != SEM_FAILED) {
    if (sem_close(sem_r_id) == -1) {
      warn("sem_close()");
    }
    if (sem_unlink(sem_r_name) == -1) {
      warn("sem_unlink()");
    }
  }

  if (shm_buffer != MAP_FAILED) {
    if (munmap(shm_buffer, (size_t)global_shm_size) == -1) {
      warn("munmap()");
    }
  }

  if (shm_fd != -1) {
    if (close(shm_fd) == -1) {
      warn("close()");
    }
    if (shm_unlink(shm_name) == -1) {
      warn("shm_unlink()");
    }
  }
}

/**
 * @brief handles signals by performing a cleanup before exit
 *
 * @param signum the signal number
 */
void shared_signal(int signum) {
  shared_cleanup();
  _exit(signum);
}

/**
 * @brief writes data from the stream to a ring buffer-based shared memory
 *
 * @param shm_size the size of the shared memory
 * @param stream the stream to be processed
 *
 * @returns 0 if everything went well and -1 in case of error
 */
int shared_send(long shm_size, FILE *stream) {
  int input;
  int position = 0;

  if (shared_init(shm_size) == -1) {
    shared_cleanup();
    /* error is printed by shared_init() */
    return -1;
  }

  do {
    /* decrement the free space and wait if 0 */
    while (sem_wait(sem_w_id) == -1) {
      /* try again after a signal interrupt */
      if (errno == EINTR) {
        continue;
      } else {
        warn("sem_wait()");
        shared_cleanup();
        return -1;
      }
    }

    input = fgetc(stream);

    if (input == EOF && ferror(stream) != 0) {
      warn("fgetc()");
      shared_cleanup();
      return -1;
    }

    shm_buffer[position] = input;
    position++;

    /* wrap around the ring buffer */
    if (position == shm_size) {
      position = 0;
    }

    /* increment the number of characters */
    if (sem_post(sem_r_id) == -1) {
      warn("sem_post()");
      shared_cleanup();
      return -1;
    }
  } while (input != EOF);

  return 0;
}

/**
 * @brief prints data from the ring buffer-based shared memory
 *
 * @param shm_size the size of the shared memory
 *
 * @returns 0 if everything went well and -1 in case of error
 */
int shared_receive(long shm_size) {
  int output;
  int position = 0;

  if (shared_init(shm_size) == -1) {
    shared_cleanup();
    /* error is printed by shared_init() */
    return -1;
  }

  do {
    /* decrement the number of characters and wait if 0 */
    while (sem_wait(sem_r_id) == -1) {
      /* try again after a signal interrupt */
      if (errno == EINTR) {
        continue;
      } else {
        warn("sem_wait()");
        shared_cleanup();
        return -1;
      }
    }

    output = shm_buffer[position];

    if (output != EOF) {
      if (printf("%c", output) < 0) {
        warn("printf()");
        shared_cleanup();
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
      warn("sem_post()");
      shared_cleanup();
      return -1;
    }
  } while (output != EOF);

  shared_cleanup();

  return 0;
}
