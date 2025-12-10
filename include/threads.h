#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>


char buffer[8192];
void *read_from_dial_thread(void *args);

void *write_to_dial_thread(void *args);


void perror_exit(const char *msg);