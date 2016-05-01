#ifndef _SENDER_RECEIVER_H_
#define _SENDER_RECEIVER_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "../lib/sem182/sem182.h"

void generate_keys(void);
void init(void);

#endif /* _SENDER_RECEIVER_H_ */
