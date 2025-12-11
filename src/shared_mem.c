#include "../include/shared_mem.h"

shared_mem *create_shared_memory(void){
    int fd;
    shared_mem  *shmp;
    bool first_process = 0;

    fd = shm_open(SHM_PATH, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd == -1 && errno == EEXIST){
        
        fd = shm_open(SHM_PATH, O_RDWR, 0600);
        if (fd == -1)
                errExit("shm_open existing");
    }
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

    if(first_process){

        memset(shmp, 0, sizeof(shared_mem));   // MUST BE HERE FIRST
        shmp->proc = 1;
        
        for (int i = 0; i < MAX_DIALOGS; i++) {
            Dialog *d = &shmp->dialogs[i];

            // semaphore protecting dialog state
            if (sem_init(&d->mutex, 1, 1) == -1)
                errExit("sem_init mutex");

            // empty = 1 → μπορούμε να γράψουμε νέο μήνυμα
            if (sem_init(&d->empty, 1, 1) == -1)
                errExit("sem_init empty");

            for (int j = 0; j < MAX_PROCS; j++) {
                if (sem_init(&d->can_read[j], 1, 0) == -1)
                    errExit("sem_init can_read");
            }
            d->active = 0;
            d->dialog_id = -1;
        }


    }
    else{
        shmp->proc++;
    } 
    
    close(fd);
    return shmp;
}