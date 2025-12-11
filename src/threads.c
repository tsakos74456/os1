#include "../include/threads.h"


void perror_exit(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}


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
        int ret = poll(fds, 2, -1);   // <-- REQUIRED
        if (ret < 0) {
            perror("poll");
            break;
        }

        if(fds[1].revents & POLLIN){
            if (read(fds[1].fd,read_buffer,1) == 1)
                if((signed char)read_buffer[0] == -1)
                    break;
        }

        if (fds[0].revents & POLLIN) {
            if(fgets(read_buffer, 8192, stdin) != NULL  ) {
                read_buffer[strcspn(read_buffer, "\n")] = '\0';

                if(sem_wait(&d->can_send_mess) == -1)
                        errExit("sem_wait");  

                if(sem_wait(&d->dial_mutex) == -1)
                    errExit("sem_wait"); 

                d->message.sender_pid = getpid();
                d->message.readers_total = 0;
                memcpy(d->message.payload,read_buffer,strlen(read_buffer) + 1);

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

        // don't read till everyone does and don't let the proc read its own message
        if (getpid() == d->message.sender_pid) {

            d->message.readers_total++;
            

            if(!strcmp(d->message.payload,"TERMINATE"))
                terminated = 1;

            
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
        

        memcpy(buffer,d->message.payload,strlen(d->message.payload) + 1);

        d->message.readers_total++;
        
        if(!strcmp(buffer,"TERMINATE"))
            terminated = 1;

        if(d->participant_count == d->message.readers_total){
            // after the read is finished clear buffer
            d->message.payload[0] = '\0';

            if(sem_post(&d->can_send_mess) == -1)
                    errExit("sem_post");
            
        }

        if(sem_post(&d->dial_mutex) == -1)
            errExit("sem_post");

        printf("%s\n",buffer);
        fflush(stdout);
        if (terminated) {
            signed char c = -1;
            write(t_args->wake_pipe[1],&c,1);
        }

    }
        
    free(buffer);
    return NULL;
}