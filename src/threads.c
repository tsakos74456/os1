#include "../include/threads.h"


void perror_exit(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

// stdin -> shared memory
void *read_from_dial_thread(void *args){
    thread_args *t_args = args;
    Dialog *d = &t_args->shmp->dialogs[t_args->dial_idx];

    char *read_buffer = malloc (8192 * sizeof(char));
    if(read_buffer == NULL){
        printf("Failed memory allocation!\n");
        return NULL;
    }
    bool terminated = 0;


    // set poll in order to solve the problem in which fgets blocks so it cannot stop when terminate is written
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN; // read
    
    fds[1].fd = t_args->wake_pipe[0];
    fds[1].events = POLLIN;

    while (!terminated){
        int ret = poll(fds, 2, -1); 
        if (ret < 0) {
            perror("poll");
            break;
        }

        // if the write thread reads terminate it sends a flag of -1 in order to terminate the read thread
        if(fds[1].revents & POLLIN){
            if (read(fds[1].fd,read_buffer,1) == 1)
                if((signed char)read_buffer[0] == -1)
                    break;
        }

        // read from stdin
        if (fds[0].revents & POLLIN) {
            if(fgets(read_buffer, 8192, stdin) != NULL  ) {
                // replace change of line with null
                read_buffer[strcspn(read_buffer, "\n")] = '\0';

                // lock the mutexes and copy the message to shared memory
                if(sem_wait(&d->can_send_mess) == -1)
                        errExit("sem_wait");  

                if(sem_wait(&d->dial_mutex) == -1)
                    errExit("sem_wait"); 

                d->message.sender_pid = getpid();
                d->message.readers_total = 0;
                memcpy(d->message.payload,read_buffer,strlen(read_buffer) + 1);

                // "wake up" the semaphores for each participant in this dialog
                for (int i = 0; i < d->participant_count ; i++)
                    if(sem_post(&d->can_be_read[i]) == -1)
                        errExit("sem_post"); 

                if(sem_post(&d->dial_mutex) == -1)
                        errExit("sem_post");
                
            }
        }

    }
    free(read_buffer);
    return NULL;

}

void *write_to_dial_thread(void *args){
    thread_args *t_args = args;
    Dialog *d = &t_args->shmp->dialogs[t_args->dial_idx];

    char *buffer = malloc(8192 * sizeof(char));
    if(buffer == NULL){
        printf("Failed memory allocation!\n");
        return NULL;
    }
    
    bool terminated = 0;

    while (!terminated) {

        if(sem_wait(&d->can_be_read[t_args->my_index]) == -1)
            errExit("sem_wait");

        if(sem_wait(&d->dial_mutex) == -1)
            errExit("sem_wait");

        // if u this process is the one which sends the message don't print it just update the struct
        if (getpid() == d->message.sender_pid) {

            // increase the value which shows how many read it, check for terminate  
            d->message.readers_total++;
            

            if(!strcmp(d->message.payload,"TERMINATE"))
                terminated = 1;

            // if u are the last one to read the message post the sem of empty message
            if(d->participant_count == d->message.readers_total){
                // after the read is finished clear buffer
                d->message.payload[0] = '\0';
                if(sem_post(&d->can_send_mess) == -1)
                    errExit("sem_post");
            }
            
            if(sem_post(&d->dial_mutex) == -1)
                    errExit("sem_post");
            
            if (terminated) {
                signed char c = -1;
                write(t_args->wake_pipe[1],&c,1);
            }
                  
            continue;  
        }
        
        // u are not the process which sent the message so print and update
        memcpy(buffer,d->message.payload,strlen(d->message.payload) + 1);

        d->message.readers_total++;
        
        if(!strcmp(buffer,"TERMINATE"))
            terminated = 1;

        // if u are the last one to read the message post the sem of empty message
        if(d->participant_count == d->message.readers_total){
            // after the read is finished clear buffer
            d->message.payload[0] = '\0';

            if(sem_post(&d->can_send_mess) == -1)
                    errExit("sem_post");
            
        }

        if(sem_post(&d->dial_mutex) == -1)
            errExit("sem_post");

        printf("Message received: %s\n",buffer);
        fflush(stdout);

        // if u received message of termination just sent to the read thread the flag through the pipe
        if (terminated) {
            signed char c = -1;
            write(t_args->wake_pipe[1],&c,1);
        }

    }
        
    free(buffer);
    return NULL;
}