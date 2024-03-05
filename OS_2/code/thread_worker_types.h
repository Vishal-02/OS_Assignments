#ifndef TW_TYPES_H
#define TW_TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <ucontext.h>

typedef unsigned int worker_t;

enum State {
    WAITING,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
};

typedef struct TCB
{
    /* add important states in a thread control block */
    // thread Id
    // thread status
    // thread context
    // thread stack
    // thread priority
    // An+d more ...

    // YOUR CODE HERE
    worker_t thread_id;
    enum State thread_status;
    ucontext_t context;
    int priority;
    void* value;

} tcb;


#endif
