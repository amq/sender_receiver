#include "shared.h"

#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
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
  sem_t *sem_w_id;
  sem_t *sem_r_id;
  long shm_size;
  int shm_fd;
  int *shm_buffer;
} shared_t;

/* a global pointer, used for signal handling only */
static shared_t *data_ptr = NULL;

/* internal function prototypes */
static int shared_init(long shm_size, shared_t *data);
static void shared_cleanup(shared_t *data);
static void shared_close(shared_t *data);
static void shared_remove(shared_t *data);
static void shared_signal(int signum);

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
      /* over-/underflow or some characters could not be converted or too small */
      if (errno != 0 || *notconv != '\0' || shm_size < 1) {
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
 * @brief writes data from the stream to a ring buffer-based shared memory
 *
 * @param shm_size the size of the shared memory
 * @param stream the stream to be processed
 *
 * @returns 0 if everything went well and -1 in case of error
 */
int shared_send(long shm_size, FILE *stream) {
  shared_t data;
  int input = EOF;
  int position = 0;

  if (shared_init(shm_size, &data) == -1) {
    /* error is printed by shared_init() */
    shared_cleanup(&data);
    return -1;
  }

  do {
    /* decrement the free space and wait if 0 */
    if (sem_wait(data.sem_w_id) == -1) {
      /* try again after a signal interrupt */
      if (errno == EINTR) {
        continue;
      } else {
        warn("sem_wait");
        shared_cleanup(&data);
        return -1;
      }
    }

    if (data.shm_buffer[data.shm_size] == EOF) {
      warnx("Receiver exited unexpectedly");
      shared_close(&data);
      return -1;
    }

    input = fgetc(stream);

    if (input == EOF && ferror(stream) != 0) {
      warn("fgetc");
      shared_cleanup(&data);
      return -1;
    }

    data.shm_buffer[position] = input;
    position++;

    /* wrap around the ring buffer */
    if (position == shm_size) {
      position = 0;
    }

    /* increment the number of characters */
    if (sem_post(data.sem_r_id) == -1) {
      warn("sem_post");
      shared_cleanup(&data);
      return -1;
    }
  } while (input != EOF);

  shared_close(&data);

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
  shared_t data;
  int output = EOF;
  int position = 0;

  if (shared_init(shm_size, &data) == -1) {
    /* error is printed by shared_init() */
    shared_cleanup(&data);
    return -1;
  }

  do {
    /* decrement the number of characters and wait if 0 */
    if (sem_wait(data.sem_r_id) == -1) {
      /* try again after a signal interrupt */
      if (errno == EINTR) {
        continue;
      } else {
        warn("sem_wait");
        shared_cleanup(&data);
        return -1;
      }
    }

    if (data.shm_buffer[data.shm_size] == EOF) {
      warnx("Sender exited unexpectedly");
      shared_close(&data);
      return -1;
    }

    output = data.shm_buffer[position];
    position++;

    if (output != EOF) {
      if (printf("%c", output) < 0) {
        warn("printf");
        shared_cleanup(&data);
        return -1;
      }
    }

    /* wrap around the ring buffer */
    if (position == shm_size) {
      position = 0;
    }

    /* increment the free space */
    if (sem_post(data.sem_w_id) == -1) {
      warn("sem_post");
      shared_cleanup(&data);
      return -1;
    }
  } while (output != EOF);

  shared_cleanup(&data);

  return 0;
}

/**
 * @brief creates or opens the semaphores and shared memory
 *
 * @param shm_size the desired size of the shared memory
 * @param data the struct to contain the ipc names and ids
 *
 * @returns 0 if everything went well and -1 in case of error
 */
static int shared_init(long shm_size, shared_t *data) {
  data->shm_size = shm_size;
  data->shm_fd = -1;
  data->shm_buffer = MAP_FAILED;
  data->sem_w_id = SEM_FAILED;
  data->sem_r_id = SEM_FAILED;

  /* save the pointer into a global variable for signal handling */
  data_ptr = data;

  /* register the signal handler */
  struct sigaction sa;
  sa.sa_handler = &shared_signal;

  /* restart system calls instead of falling into EINTR */
  sa.sa_flags = SA_RESTART;

  /* block signals during the handling */
  sigfillset(&sa.sa_mask);

  /* intercept termination signals */
  if (sigaction(SIGINT, &sa, NULL) == -1) {
    warn("sigaction");
    return -1;
  }
  if (sigaction(SIGTERM, &sa, NULL) == -1) {
    warn("sigaction");
    return -1;
  }
  if (sigaction(SIGHUP, &sa, NULL) == -1) {
    warn("sigaction");
    return -1;
  }

  /* convert the sem/shm number (as defined in spec) to a string */
  if (sprintf(data->shm_name, "%c%llu", '/', IPC_NAME(0)) < 0) {
    warn("sprintf");
    return -1;
  }
  if (sprintf(data->sem_w_name, "%c%llu", '/', IPC_NAME(1)) < 0) {
    warn("sprintf");
    return -1;
  }
  if (sprintf(data->sem_r_name, "%c%llu", '/', IPC_NAME(2)) < 0) {
    warn("sprintf");
    return -1;
  }

  /* open a shared memory object */
  if ((data->shm_fd = shm_open(data->shm_name, O_RDWR | O_CREAT, IPC_PERMS)) == -1) {
    warn("shm_open");
    return -1;
  }

  /* set the size of the shared memory object and initialize it with 0 */
  if (ftruncate(data->shm_fd, shm_size * sizeof(*data->shm_buffer) + 1) == -1) {
    warn("ftruncate");
    return -1;
  }

  /* map the memory object so that it can be used, similar to malloc */
  if ((data->shm_buffer = mmap(NULL, shm_size * sizeof(*data->shm_buffer) + 1,
                               PROT_READ | PROT_WRITE, //
                               MAP_SHARED, data->shm_fd, 0)) == MAP_FAILED) {
    warn("mmap");
    return -1;
  }

  /* write semaphore: initially the free space is equal to shm_size */
  if ((data->sem_w_id = sem_open(data->sem_w_name, O_CREAT, IPC_PERMS, shm_size)) == SEM_FAILED) {
    warn("sem_open");
    return -1;
  }

  /* read semaphore: initially there are no characters to be read */
  if ((data->sem_r_id = sem_open(data->sem_r_name, O_CREAT, IPC_PERMS, 0)) == SEM_FAILED) {
    warn("sem_open");
    return -1;
  }

  return 0;
}

/**
 * @brief performs a full cleanup
 *
 * @param data the struct which contains the ipc names and ids
 */
static void shared_cleanup(shared_t *data) {
  /* notify the other process over an unused element in buffer */
  data->shm_buffer[data->shm_size] = EOF;

  /* unlock the semaphores so that the other process can terminate */
  if (data->sem_w_id != SEM_FAILED) {
    if (sem_post(data->sem_w_id) == -1) {
      warn("sem_post");
    }
  }
  if (data->sem_r_id != SEM_FAILED) {
    if (sem_post(data->sem_r_id) == -1) {
      warn("sem_post");
    }
  }

  shared_close(data);
  shared_remove(data);
}

/**
 * @brief closes the semaphores and shared memory
 *
 * @param data the struct which contains the ipc names and ids
 */
static void shared_close(shared_t *data) {
  if (data->sem_w_id != SEM_FAILED) {
    if (sem_close(data->sem_w_id) == -1) {
      warn("sem_close");
    }
  }
  if (data->sem_r_id != SEM_FAILED) {
    if (sem_close(data->sem_r_id) == -1) {
      warn("sem_close");
    }
  }
  if (data->shm_buffer != MAP_FAILED) {
    if (munmap(data->shm_buffer, (size_t)data->shm_size) == -1) {
      warn("munmap");
    }
  }
}

/**
 * @brief removes the semaphores and shared memory
 *
 * @param data the struct which contains the ipc names and ids
 */
static void shared_remove(shared_t *data) {
  if (data->sem_w_id != SEM_FAILED) {
    if (sem_unlink(data->sem_w_name) == -1) {
      warn("sem_unlink");
    }
  }
  if (data->sem_r_id != SEM_FAILED) {
    if (sem_unlink(data->sem_r_name) == -1) {
      warn("sem_unlink");
    }
  }
  if (data->shm_fd != -1) {
    if (shm_unlink(data->shm_name) == -1) {
      warn("shm_unlink");
    }
  }
}

/**
 * @brief handles signals by performing a cleanup before exit
 *
 * @param signum the signal number
 */
static void shared_signal(int signum) {
  shared_cleanup(data_ptr);
  _exit(signum);
}
