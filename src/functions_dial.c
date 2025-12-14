#include "../include/functions_dial.h"


int enter_dialog(shared_mem *shmp, const int dial_id, thread_args *t_args){
    
    // if dialog already exists find it
    for(int real_pos = 0 ; real_pos < MAX_DIALOGS ; real_pos++){
        Dialog *d = &shmp->dialogs[real_pos];
        if(sem_wait(&d->dial_mutex) == -1){
            errExit("sem_wait");
        }

        if(d->active && d->dialog_id == dial_id){
            
            // found insert the participant
            if(d->participant_count < MAX_PROCS){
                t_args->my_index = d->participant_count;
                d->participant_pids[d->participant_count++] = getpid();
                if(sem_post(&d->dial_mutex) == -1)
                    errExit("sem_post");
                return real_pos;
            }
            // not enough space for participant
            else{
                
                if(sem_post(&d->dial_mutex) == -1)
                    errExit("sem_post");
                return -1;
            }
        }
        if(sem_post(&d->dial_mutex) == -1)
            errExit("sem_post");
    }

    // create the new dialog and initialize it
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
    // there is not any available slot for the dialog
    return -1;
}

bool destroy_dialogues(thread_args *t_args){
    Dialog *d = &t_args->shmp->dialogs[t_args->dial_idx];

    if(sem_wait(&d->dial_mutex) == -1)
        errExit("sem_wait");

    d->participant_count--;
    // if you are the last active participant set dialog inactive
    if(d->participant_count == 0){
        d->active = 0;
        d->dialog_id = -1;
    } 

    if(sem_post(&d->dial_mutex) == -1)
            errExit("sem_post");
    bool last_one = 0;
    if(sem_wait(&t_args->shmp->shmp_mutex) == -1)
            errExit("sem_wait");
    
    t_args->shmp->proc--;
    if(t_args->shmp->proc == 0)
        last_one = 1;    
    
    if(sem_post(&t_args->shmp->shmp_mutex) == -1)
        errExit("sem_post");



    return last_one;
}
