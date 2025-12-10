#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "../include/threads.h"
#include <unistd.h>
#include "../include/shared_mem.h"
#include <ctype.h>
#define SHM_PATH "/dialogs_shm"

// USE : ./os <num of dialog to participate>
    pthread_mutex_t mutex;
    pthread_cond_t cond;

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("Incorrect args");
        return -1;
    }

    // initialization and allocation of our needs
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("pthread_mutex_init failed in create admin\n");
    }

    if (pthread_cond_init(&cond, NULL) != 0) {
        pthread_mutex_destroy(&mutex);
        printf("pthread_cond_init failed in create admin\n");
    }

    int dial_id = atoi(argv[1]);
    
    shared_mem  *shmp = create_shared_memory();




    // threads for simultaneous communication among the processes 
    pthread_t *tids;
    tids = malloc(2 * sizeof(pthread_t));

    // thread stdin -> dialog
    if(pthread_create(&tids[0],NULL,read_from_dial_thread,NULL) != 0)
        perror_exit("pthread_create");

    // thread dialog->stdout
    if(pthread_create(&tids[1],NULL,write_to_dial_thread,NULL) != 0)
        perror_exit("pthread_create");



    pthread_join(tids[0],NULL);
    pthread_join(tids[1],NULL);
    free(tids);
}
