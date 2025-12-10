#include "../include/shared_mem.h"

shared_mem *create_shared_memory(void){
    int fd;
    shared_mem  *shmp;
    fd = shm_open(SHM_PATH, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd == -1 && errno == EEXIST){
        fd = shm_open(SHM_PATH, O_RDWR, 0600);
        if (fd == -1)
                errExit("shm_open existing");
    }
    else{
        if (ftruncate(fd, sizeof(shared_mem)) == -1)
            errExit("ftruncate");}

    /* Map the object into the caller's address space. */

    shmp = mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE,
                MAP_SHARED, fd, 0);
    if (shmp == MAP_FAILED)
        errExit("mmap");

    close(fd);
    return shmp;
}