#include "../include/shared_mem.h"

shared_mem *create_shared_memory(void){
    int fd;
    shared_mem  *shmp;
    bool first_process = 0;

    // BUILD SHARED MEMORY  
    fd = shm_open(SHM_PATH, O_CREAT | O_EXCL | O_RDWR, 0600);
    // if it exists
    if (fd == -1 && errno == EEXIST){
        
        fd = shm_open(SHM_PATH, O_RDWR, 0600);
        if (fd == -1)
                errExit("shm_open existing");
    }
    // create it and set that this is the 1st process so it has to initialize everything
    else{
        first_process = 1;
        if (ftruncate(fd, sizeof(shared_mem)) == -1)
            errExit("ftruncate");
    }

    /* Map the object into the caller's address space. */

    shmp = mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE,
                MAP_SHARED, fd, 0);
    if (shmp == MAP_FAILED)
        errExit("mmap");

    // initialize semaphores and shared mem
    if(first_process){

        memset(shmp, 0, sizeof(shared_mem));  

        shmp->proc = 1;

        if (sem_init(&shmp->shmp_mutex, 1, 1) == -1)
                errExit("sem_init empty");
                        
        for (int i = 0; i < MAX_DIALOGS; i++) {
            Dialog *d = &shmp->dialogs[i];

            // semaphore protecting dialog state
            if (sem_init(&d->dial_mutex, 1, 1) == -1)
                errExit("sem_init mutex");

            if (sem_init(&d->can_send_mess, 1, 1) == -1)
                errExit("sem_init empty");

            for (int j = 0; j < MAX_PROCS; j++) {
                if (sem_init(&d->can_be_read[j], 1, 0) == -1)
                    errExit("sem_init can_read");
            }

            if(sem_wait(&d->dial_mutex) == -1)
                errExit("sem_wait");

            d->active = 0;
            d->dialog_id = -1;

            if(sem_post(&d->dial_mutex) == -1)
                errExit("sem_post");
        }

    }
    // increase the amount of total active participants in dialogs
    else{

        if(sem_wait(&shmp->shmp_mutex) == -1)
            errExit("sem_wait");

        shmp->proc++;
        if(sem_post(&shmp->shmp_mutex) == -1)
            errExit("sem_post");
        
    } 
    
    close(fd);
    return shmp;
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