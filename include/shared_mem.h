#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>

#define SHM_PATH "/dialogs_shm"
#define MAX_PROCS 16 //16 participants per dialog
#define MAX_DIALOGS 16 //8 DIALOG
#define MAX_PAYLOAD 8092
#define MAX_MESSAGES 8

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)
// message
typedef struct {
    bool read_by_everyone;
    int sender_pid;
    char payload[MAX_PAYLOAD];
    int readers_total;
} Message;


// struct for dialog
typedef struct { 
    bool active;               // 1 = dialog active, 0 = dialog inactive

    int dialog_id;

    Message messages[MAX_MESSAGES];

    int participant_pids[MAX_PROCS];
    int participant_count;     //number of particiapants in this dialog
} Dialog;


typedef struct {
    Dialog dialogs[MAX_DIALOGS];

    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} shared_mem;

shared_mem *create_shared_memory(void);