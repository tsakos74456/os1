#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../include/threads.h"
#include <unistd.h>
#include <ctype.h>
#define SHM_PATH "/dialogs_shm"

// USE : ./os <num of dialog to participate>
int enter_dialog(shared_mem *shmp,const int id, thread_args *t_args);
void destroy_dialogues(thread_args *t_args);
void destroy_all(shared_mem *shmp);

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
    
    if(pipe(t_args->wake_pipe)){
        printf("Problem with pipe\n");
        return -2;
    }
    


    t_args->dial_idx = enter_dialog(shmp, arg_dial_id,t_args);
    if(t_args->dial_idx == -1){
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


    destroy_dialogues(t_args);
    close(t_args->wake_pipe[0]);
    close(t_args->wake_pipe[1]);

    free(t_args);
    free(tids);
}


int enter_dialog(shared_mem *shmp, const int dial_id, thread_args *t_args){
    
    // if it already exists find the dialog
    for(int real_pos = 0 ; real_pos < MAX_DIALOGS ; real_pos++){
        Dialog *d = &shmp->dialogs[real_pos];
        if(sem_wait(&d->dial_mutex) == -1){
            errExit("sem_wait");
        }

        if(d->active && d->dialog_id == dial_id){
            
            if(d->participant_count < MAX_PROCS){
                t_args->my_index = d->participant_count;
                d->participant_pids[d->participant_count++] = getpid();
                if(sem_post(&d->dial_mutex) == -1)
                    errExit("sem_post");
                return real_pos;
            }
            else{
                if(sem_post(&d->dial_mutex) == -1)
                    errExit("sem_post");
                return -1;
            }
        }
        if(sem_post(&d->dial_mutex) == -1)
            errExit("sem_post");
    }

    for(int i = 0 ; i < MAX_DIALOGS ; i++){

        Dialog *d = &shmp->dialogs[i];

        if(sem_wait(&d->dial_mutex) == -1)
            errExit("sem_wait");
        

        if(!(d->active)){
            d->dialog_id = dial_id;
            d->active = 1;
            d->participant_pids[0] = getpid();
            t_args->my_index = 0;
            d->participant_count = 1;

            // initialize messages
            d->message.payload[0] = '\0';
            d->message.readers_total = 0;
            d->message.sender_pid = -1;
            
            if(sem_post(&d->dial_mutex) == -1)
                errExit("sem_post");
                
            return i;
        }
        if(sem_post(&d->dial_mutex) == -1)
            errExit("sem_post");

    }
    return -1;
}

void destroy_dialogues(thread_args *t_args){
    
    if(sem_wait(&t_args->shmp->shmp_mutex) == -1)
            errExit("sem_wait");
    
    t_args->shmp->proc--;
    const int procs_remaining = t_args->shmp->proc;

    if(sem_post(&t_args->shmp->shmp_mutex) == -1)
        errExit("sem_post");
    
    Dialog *d = &t_args->shmp->dialogs[t_args->dial_idx];

    if(sem_wait(&d->dial_mutex) == -1)
        errExit("sem_wait");

    d->participant_count--;
    if(d->participant_count == 0){
        d->active = 0;
        d->dialog_id = -1;
    } 

    if(sem_post(&d->dial_mutex) == -1)
            errExit("sem_post");

    if(procs_remaining == 0)
        destroy_all(t_args->shmp);

    return;
}

void destroy_all(shared_mem *shmp){
    for(int i = 0 ; i < MAX_DIALOGS ; i++){
        Dialog *d = &shmp->dialogs[i];
        sem_destroy(&d->can_send_mess);
        sem_destroy(&d->dial_mutex);
        
        
        for(int j = 0 ; j < MAX_PROCS ; j++)
            sem_destroy(&d->can_be_read[j]);
    }
    sem_destroy(&shmp->shmp_mutex);
    munmap(shmp,sizeof(shared_mem));
    shm_unlink(SHM_PATH);

    return;
}