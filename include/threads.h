#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "shared_mem.h"
#include <poll.h>

typedef struct {
    int dial_idx; //index in dialogues
    shared_mem *shmp;
    int my_index;  //index in pids_participants
    int wake_pipe[2];
} thread_args;


// thread stdin -> dialog
void *read_from_dial_thread(void *args);


// thread dialog->stdout
void *write_to_dial_thread(void *args);


void perror_exit(const char *msg);