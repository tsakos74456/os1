#include "../include/threads.h"
#include "../include/shared_mem.h"


void perror_exit(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}


void *read_from_dial_thread(void *args){

    char *read_buffer = malloc (8192 * sizeof(char));
    while (fgets(read_buffer, 8192, stdin) != NULL) {
        
        read_buffer[strcspn(read_buffer, "\n")] = '\0';


        pthread_mutex_lock(&mutex);
        strcpy(buffer,read_buffer);
        new_message = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);

        if(!strcmp(buffer,"TERMINATE"))
            break;
    }

    free(read_buffer);
}

void *write_to_dial_thread(void *args){

    while (1) {
        pthread_mutex_lock(&mutex);
        while (!new_message)
            pthread_cond_wait(&cond,&mutex);

        new_message = 0;
        printf("%s\n",buffer);
        pthread_mutex_unlock(&mutex);
        if(!strcmp(buffer,"TERMINATE"))
            break;
    }
}