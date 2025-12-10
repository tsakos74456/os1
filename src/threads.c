#include "../include/threads.h"


void perror_exit(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}


void *read_from_dial_thread(void *args){
    thread_args *t_args = args;
    Dialog *d = &t_args->shmp->dialogs[t_args->dial_id];

    char *read_buffer = malloc (8192 * sizeof(char));
    if(read_buffer == NULL){
        printf("Failed memory allocation!\n");
        return NULL;
    }

    
    while (fgets(read_buffer, 8192, stdin) != NULL) {
        
        read_buffer[strcspn(read_buffer, "\n")] = '\0';

        sem_wait(&d->empty);   
        sem_wait(&d->mutex);  

        d->message.readers_total = 0;
        d->message.sender_pid = getpid();
        d->message.message_id++;

        memcpy(d->message.payload,read_buffer,strlen(read_buffer) + 1);

        

        sem_post(&d->mutex); 
        for (int i = 0; i < d->participant_count ; i++)
            sem_post(&d->full);   
        

    }

    free(read_buffer);
    return NULL;

}

void *write_to_dial_thread(void *args){
    thread_args *t_args = args;
    Dialog *d = &t_args->shmp->dialogs[t_args->dial_id];

    char *buffer = malloc(8192 * sizeof(char));
    if(buffer == NULL){
        printf("Failed memory allocation!\n");
        return NULL;
    }

    bool terminated = 0;
    
    // we use the last message id to maek sure someone will not read a mesage twice 
    int last_message_id = -1;

    while (1) {

        
        sem_wait(&d->mutex);

        if(last_message_id == d->message.message_id){
            sem_post(&d->mutex);
            continue;
        }
        sem_wait(&d->full); 
        last_message_id = d->message.message_id;

        // don't read till everyone does and don't let the proc read its own message
        if (getpid() == d->message.sender_pid) {

            d->message.readers_total++;
            
            if(d->participant_count == d->message.readers_total )
                sem_post(&d->empty);

            if(!strcmp(d->message.payload,"TERMINATE"))
                terminated = 1;

            sem_post(&d->mutex);      
            
            if (terminated)
                break;
                  
            continue;  
        }
        

        memcpy(buffer,d->message.payload,strlen(d->message.payload) + 1);

        d->message.readers_total++;
        if(d->participant_count == d->message.readers_total )
            sem_post(&d->empty);
            
        
        if(!strcmp(d->message.payload,"TERMINATE"))
            terminated = 1;

        sem_post(&d->mutex);

        printf("%s\n",buffer);
        fflush(stdout);

        if(terminated)
            break;
    }

    free(buffer);
    return NULL;
}