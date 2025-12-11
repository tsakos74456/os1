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
#define MAX_PROCS 16 //16 participants per dialog
#define MAX_DIALOGS 16 //8 DIALOG
#define MAX_PAYLOAD 8192

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)
// message
typedef struct {
    int sender_pid;
    char payload[MAX_PAYLOAD];
    int readers_total;
} Message;


// struct for dialog
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


typedef struct {
    Dialog dialogs[MAX_DIALOGS];
    int proc;  //numbe of participants in dials (total)
    sem_t shmp_mutex; //protects shared memory
} shared_mem;

shared_mem *create_shared_memory();