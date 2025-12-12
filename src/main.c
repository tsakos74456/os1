#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../include/functions_dial.h"
#include <unistd.h>
#include <ctype.h>
#define SHM_PATH "/dialogs_shm"

// USE : ./os <num of dialog to participate>

int main(int argc, char *argv[]){

    // check args
    if(argc != 2){
        printf("Incorrect args");
        return -1;
    }
    int arg_dial_id = atoi(argv[1]);

    // initialization and memory allocation so we can pass the needed values to the threads
    shared_mem  *shmp = create_shared_memory();
    
    thread_args *t_args = malloc(sizeof(thread_args));
    if(t_args == NULL){
        printf("Failed memory allocation!\n");
        return -1;
    }
    
    // use pipe cause we want to be able to communicate between the write and read thread for the TERMINATE
    if(pipe(t_args->wake_pipe)){
        printf("Problem with pipe\n");
        return -2;
    }
    
    // enter a dialog
    t_args->dial_idx = enter_dialog(shmp, arg_dial_id,t_args);
    if(t_args->dial_idx == -1){
        printf("The dialogues or participants are full! Sorry pls try again later.\n");
        return -1;
    }
    t_args->shmp  = shmp;

    // threads for simultaneous communication among the processes (read/write message)
    pthread_t *tids;
    tids = malloc(2 * sizeof(pthread_t));

    // thread stdin -> dialog
    if(pthread_create(&tids[0],NULL,read_from_dial_thread,t_args) != 0)
        perror_exit("pthread_create");

    // thread dialog->stdout
    if(pthread_create(&tids[1],NULL,write_to_dial_thread,t_args) != 0)
        perror_exit("pthread_create");

    // threads join and destroy everything so we dont have leaks
    pthread_join(tids[0],NULL);
    pthread_join(tids[1],NULL);


    destroy_dialogues(t_args);
    close(t_args->wake_pipe[0]);
    close(t_args->wake_pipe[1]);

    free(t_args);
    free(tids);
}
