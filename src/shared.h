#pragma once

#include <stdio.h>
#include <stdlib.h>

long shared_parse_size(int argc, char *argv[]);
int shared_send(long shm_size, FILE *stream);
int shared_receive(long shm_size);
