#include "shared.h"

/* write semaphore: free space */
sem_t *sem_w_id = SEM_FAILED;

/* read semaphore: number of characters left to be read */
sem_t *sem_r_id = SEM_FAILED;

int shm_id = -1;
int *shm_address = MAP_FAILED;

/**
 * @brief
 *
 * @param
 *
 * @returns
 */
long parse_shm_size(int argc, char *argv[]) {
  int opt;
  long shm_size = -1;
  char *notconv = '\0';

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

  /* there were some non-option arguments */
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
  char sem_w_name[NAME_LENGTH];
  char sem_r_name[NAME_LENGTH];
  char shm_name[NAME_LENGTH];

  /* convert the sem/shm number (as defined in spec) to a string */
  if (sprintf(sem_w_name, "%u", SEM_W_NAME) < 0) {
    /* errno is set by sprintf */
    return -1;
  }
  if (sprintf(sem_r_name, "%u", SEM_R_NAME) < 0) {
    return -1;
  }
  if (sprintf(shm_name, "%u", SHM_NAME) < 0) {
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

  /* open a shared memory object */
  shm_id = shm_open(shm_name, O_CREAT, S_IRUSR | S_IWUSR);

  if (shm_id == -1) {
    /* errno is set by shm_open */
    return -1;
  }

  /* set the size of the shared memory object */
  if (ftruncate(shm_id, shm_size) == -1) {
    /* errno is set by ftruncate */
    return -1;
  }

  /* map the memory object so that it can be used */
  shm_address = mmap(NULL, (size_t)shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);

  if (shm_address == MAP_FAILED) {
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
int cleanup(long shm_size) {
  errno = 0;

  sem_close(sem_w_id);
  sem_close(sem_r_id);
  munmap(shm_address, (size_t)shm_size);
  close(shm_id);

  /*
   * we are not doing error handling for each function separately,
   * because then resources after the function would not be closed
   */
  if (errno != 0) {
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
int write_to_shm(long shm_size, FILE *stream) {
  if (init(shm_size) == -1) {
    /* errno is set by init */
    return -1;
  }

  /* wait if there is no space left */
  while (sem_wait(sem_w_id) == -1) {
    /* try again after a signal interrupt */
    if (errno == EINTR) {
      continue;
    } else {
      return -1;
    }
  }

  /* @todo: write */

  /* increment the number of characters */
  while (sem_post(sem_r_id) == -1) {
    if (errno == EINTR) {
      continue;
    } else {
      return -1;
    }
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
int read_from_shm(int shm_size) {
  if (init(shm_size) == -1) {
    /* errno is set by init */
    return -1;
  }

  /* wait if there are no characters */
  while (sem_wait(sem_r_id) == -1) {
    if (errno == EINTR) {
      continue;
    } else {
      return -1;
    }
  }

  /* @todo: read, cleanup after EOF */

  /* increment the free space */
  while (sem_post(sem_w_id) == -1) {
    if (errno == EINTR) {
      continue;
    } else {
      return -1;
    }
  }

  return 0;
}