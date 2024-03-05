// File:	thread-worker.c

// List all group member's name:
/*
 */
// username of iLab:
// iLab Server:


#include "thread-worker.h"
#include "thread_worker_types.h"

#define STACK_SIZE 16 * 1024
#define QUANTUM 10 * 1000


// INITIALIZE ALL YOUR OTHER VARIABLES HERE
int init_scheduler_done = 0;
tcb* currently_running = NULL;
LinkedList* run_queue;
LinkedList* terminated_threads;
initializeList(run_queue);
initializeList(terminated_threads);



/* create a new thread */
int worker_create(worker_t *thread, pthread_attr_t *attr,
                  void *(*function)(void *), void *arg)
{
    // - create Thread Control Block (TCB)
    // - create and initialize the context of this worker thread
    // - allocate space of stack for this thread to run
    // after everything is set, push this thread into run queue and
    // - make it ready for the execution.
    tcb *control_block = malloc(sizeof(tcb));
    
    if (control_block == NULL) {
        perror("Failed to allocate memory to tcb");
        return -1;
    }

    void *stack = malloc(STACK_SIZE);
    if (stack == NULL) {
        perror("Failed to allocate memory for the stack");
        free(control_block);
        return -1;
    }

    if (getcontext(&control_block) == -1) {
        perror("getcontext failed");
        free(control_block);
        free(stack);
        return -1;
    }

    control_block->thread_id = *thread;
    control_block->context.uc_link=NULL;
	control_block->context.uc_stack.ss_sp=stack;
	control_block->context.uc_stack.ss_size=STACK_SIZE;
	control_block->context.uc_stack.ss_flags=0;

    makecontext(&control_block->context, (void*) function, 1, arg);
    control_block->thread_status = WAITING;

    // and now we add the thread to the queue
    insert(run_queue, thread, control_block);
    
    return 0;
}

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield()
{

    // - change worker thread's state from Running to Ready
    currently_running->thread_status = READY;

    // - save context of this thread to its thread control block
    if (getcontext(&currently_running->context) == -1) {
        perror("Failed to save context of currently running thread");
        return -1;
    }

    // - switch from thread context to scheduler context
    // gets the next thread that's in the queue
    Node* next_thread = getNext(run_queue);

    // gets the tcb of that thread
    tcb* temp = next_thread->control;
    removeNode(run_queue, temp->thread_id);

    // swaps the context, we use setcontext because we already saved context of the previous thread
    setcontext(&temp->context);

    // the new thread is now the running thread, so we change the status
    temp->thread_status = RUNNING;

    // we put the previous thread back in the run queue
    insert(run_queue, currently_running->thread_id, &currently_running);
    currently_running = temp;
    // free(temp);

    return 0;

};

/* terminate a thread */
void worker_exit(void *value_ptr)
{
    // - if value_ptr is provided, save return value
    // - de-allocate any dynamic memory created when starting this thread (could be done here or elsewhere)
    if (value_ptr != NULL) {
        currently_running->value = value_ptr;
    }

    free(currently_running->context.uc_stack.ss_sp);
    currently_running->thread_status = TERMINATED;
    insert(terminated_threads, currently_running->thread_id, currently_running);

    // now that this thread is terminated, we replace the currently running thread
    Node* next_thread = getNext(run_queue);
    removeNode(run_queue, next_thread->control->thread_id);
    currently_running = next_thread->control;

    // set the context for the new thread
    setcontext(&currently_running->context);

    return;
}

/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr)
{
    // - wait for a specific thread to terminate
    // let's get the thread
    tcb* the_thread = findNode(run_queue, thread)->control;
    while (the_thread->thread_status != TERMINATED);

    // - if value_ptr is provided, retrieve return value from joining thread
    the_thread = findNode(terminated_threads, thread)->control->value;

    // - de-allocate any dynamic memory created by the joining thread
    free(findNode(terminated_threads, thread));

    return the_thread;

};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex,
                      const pthread_mutexattr_t *mutexattr)
{
    //- initialize data structures for this mutex
    return 0;

};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex)
{

    // - use the built-in test-and-set atomic function to test the mutex
    // - if the mutex is acquired successfully, enter the critical section
    // - if acquiring mutex fails, push current thread into block list and
    // context switch to the scheduler thread
    return 0;

};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex)
{
    // - release mutex and make it available again.
    // - put one or more threads in block list to run queue
    // so that they could compete for mutex later.

    return 0;
};

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex)
{
    // - make sure mutex is not being used
    // - de-allocate dynamic memory created in worker_mutex_init

    return 0;
};

/* scheduler */
static void schedule()
{
// - every time a timer interrupt occurs, your worker thread library
// should be contexted switched from a thread context to this
// schedule() function

// - invoke scheduling algorithms according to the policy (RR or MLFQ)

// - schedule policy
#ifndef MLFQ
    // Choose RR
    
#else
    // Choose MLFQ
    
#endif
}

static void sched_rr()
{
    // - your own implementation of RR
    // (feel free to modify arguments and return types)

}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq()
{
    // - your own implementation of MLFQ
    // (feel free to modify arguments and return types)

}

// Feel free to add any other functions you need.
// You can also create separate files for helper functions, structures, etc.
// But make sure that the Makefile is updated to account for the same.