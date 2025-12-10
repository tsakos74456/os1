#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "shared_mem.h"

typedef struct {
    int dial_id;
    shared_mem *shmp;
} thread_args;


// thread stdin -> dialog
void *read_from_dial_thread(void *args);


// thread dialog->stdout
void *write_to_dial_thread(void *args);


void perror_exit(const char *msg);