#ifndef MTX_TYPES_H
#define MTX_TYPES_H

#include "thread_worker_types.h"

/* mutex struct definition */
typedef struct worker_mutex_t
{
    /* add something here */

    // YOUR CODE HERE

    int locked;
    // int locked_1;

    worker_t current_thread;
    
} worker_mutex_t;

#endif
