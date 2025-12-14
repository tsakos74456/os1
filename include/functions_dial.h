#include "threads.h"

// ESSENTIAL FUNCTIONS FOR DIALOG HANDLING

// function in order to create a dialogue or find an existing one 
// if there is not any available slot for dialog or the particiapnts have reacheed the max value returns -1
int enter_dialog(shared_mem *shmp, const int id, thread_args *t_args);

// function which destroys the dialogue when it has terminated and returns true if the process is the last one to terminate so we can destroy shared memory
bool destroy_dialogues(thread_args *t_args);