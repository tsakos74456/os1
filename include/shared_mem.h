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
#include <string.h>

#define SHM_PATH "/dialogs_shm"
#define MAX_PROCS 16 // s16 participants per dialog
#define MAX_DIALOGS 32 // 32 DIALOGUES
#define MAX_PAYLOAD 8192

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

// MESSAGE'S STRUCT
typedef struct {
    int sender_pid;
    char payload[MAX_PAYLOAD];
    int readers_total;
} Message;


// DIALOG'S STRUCT
typedef struct { 
    bool active;               // 1 = dialog active, 0 = dialog inactive
    int dialog_id;

    Message message;

    int participant_pids[MAX_PROCS];
    int participant_count;     //number of particiapants in this dialog
    
    sem_t dial_mutex; // protects dialog
    sem_t can_send_mess; // 1 when new message can be written

    sem_t can_be_read[MAX_PROCS];   // sem for each participant so it can be read by the thread
} Dialog;


// SHARED_MEMORY'S STRUCT
typedef struct {
    Dialog dialogs[MAX_DIALOGS];
    int proc;  //total number of participants in dials 
    sem_t shmp_mutex; //protects shared memory
} shared_mem;


// created the shared memory, initializes semaphores and whatever we need
shared_mem *create_shared_memory();

// destroys the dialogues  when there is noone existing and terminates
void destroy_all(shared_mem *shmp);