#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../include/threads.h"
#include <unistd.h>
#include <ctype.h>
#define SHM_PATH "/dialogs_shm"

// USE : ./os <num of dialog to participate>
int enter_dialog(shared_mem *shmp,const int id);

int main(int argc, char *argv[]){

    if(argc != 2){
        printf("Incorrect args");
        return -1;
    }
    int arg_dial_id = atoi(argv[1]);

    // initialization and allocation of our needs
    shared_mem  *shmp = create_shared_memory();
    
    thread_args *t_args = malloc(sizeof(thread_args));
    if(t_args == NULL){
        printf("Failed memory allocation!\n");
        return -1;
    }
    t_args->dial_id = enter_dialog(shmp, arg_dial_id);
    if(t_args->dial_id == -1){
        printf("Problem in entering the dialog");
        return -1;
    }
    t_args->shmp  = shmp;


    // threads for simultaneous communication among the processes 
    pthread_t *tids;
    tids = malloc(2 * sizeof(pthread_t));
    
    // thread stdin -> dialog
    if(pthread_create(&tids[0],NULL,read_from_dial_thread,t_args) != 0)
        perror_exit("pthread_create");

    // thread dialog->stdout
    if(pthread_create(&tids[1],NULL,write_to_dial_thread,t_args) != 0)
        perror_exit("pthread_create");

    // check that the last process will destroy the mutexes and free shared mem
    pthread_join(tids[0],NULL);
    pthread_join(tids[1],NULL);
    // if(shmp->proc == 0){
        // shm_unlink(SHM_PATH);
    // }

    free(t_args);
    free(tids);
}


int enter_dialog(shared_mem *shmp, const int dial_id){
    
    // if it already exists find the dialog
    for(int real_pos = 0 ; real_pos < MAX_DIALOGS ; real_pos++){
        Dialog *d = &shmp->dialogs[real_pos];
        sem_wait(&d->mutex);

        if(d->active && d->dialog_id == dial_id){
            
            if(d->participant_count < MAX_PROCS){
                d->participant_pids[d->participant_count++] = getpid();
                sem_post(&d->mutex);
                return real_pos;
            }
            else{
                sem_post(&d->mutex);
                return -1;
            }
        }
        sem_post(&d->mutex);
    }

    for(int i = 0 ; i < MAX_DIALOGS ; i++){

        Dialog *d = &shmp->dialogs[i];

        sem_wait(&d->mutex);

        if(!(d->active)){
            d->dialog_id = dial_id;
            d->active = 1;
            d->participant_pids[0] = getpid();
            d->participant_count = 1;

            // initialize messages
            d->message.payload[0] = '\0';
            d->message.readers_total = 0;
            d->message.sender_pid = -1;
            d->message.message_id = 0;
            
            sem_post(&(d->mutex));
            return i;
        }
        sem_post(&d->mutex);

    }
    return -1;
}