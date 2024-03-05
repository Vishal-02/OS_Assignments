// File:	thread-worker.c

// List all group member's name:
/*
 */
// username of iLab:
// iLab Server:


#include "thread-worker.h"
#include "thread_worker_types.h"
#include "linked_list.h"

#define STACK_SIZE 16 * 1024
#define QUANTUM 10 * 1000

#define MAXTHREADS 200

// INITIALIZE ALL YOUR OTHER VARIABLES HERE
int init_scheduler_done = 0;
int currently_running_blocked = 0;
tcb* currently_running = NULL;
LinkedList* round_robin;
LinkedList* MLFQ_level_1;
LinkedList* MLFQ_level_2;
LinkedList* MLFQ_level_3;
LinkedList* run_queue;
LinkedList* terminated_threads;
int yielded = 0; // for false


initialize(round_robin);
initialize(MLFQ_level_1);
initialize(MLFQ_level_2);
initialize(MLFQ_level_3);
initialize(run_queue);
initialize(terminated_threads);

//maintaining a list of thread that requires lock
LinkedList *block_list;
initialize(block_list); 
struct sigaction s;
struct itimerval timer;
struct timeval t;
unsigned long start_time[MAXTHREADS];
unsigned long schedule_time[MAXTHREADS];
unsigned long completion_time[MAXTHREADS];
long int response_time = 0;
long int turearound_time = 0;
ucontext_t scheduler_context;

// function declarations
static void sched_rr(LinkedList *run_queue);


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
    yielded = 1;

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
    while (the_thread->thread_status != TERMINATED) {
        printf("waiting for thread to terminate in the worker join while loop...");
    }

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

    //block_list = (LinkedList *)malloc(sizeof(LinkedList));
    if(mutex == NULL) return -1;
    mutex->locked=0;
    mutex->current_thread=NULL;

    return 0;

};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex)
{

    // - use the built-in test-and-set atomic function to test the mutex
    // - if the mutex is acquired successfully, enter the critical section
    // - if acquiring mutex fails, push current thread into block list and
    // context switch to the scheduler thread

    if(__atomic_test_and_set(&(mutex->locked),1) == 0)
    {
        currently_running->thread_status = BLOCKED;
        //insert(block_list,currently_running);
        currently_running_blocked = 1;
        swapcontext(&(currently_running->value),&scheduler_context);
    }

    mutex->current_thread = currently_running;
    mutex->locked = 1;
    return 0;

};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex)
{
    // - release mutex and make it available again.
    // - put one or more threads in block list to run queue
    // so that they could compete for mutex later.
    mutex->current_thread = NULL;
    mutex->locked = 0;
    //to move threads from blocked list to run queue
    unblock_threads();


    return 0;
};

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex)
{
    // - make sure mutex is not being used
    // - de-allocate dynamic memory created in worker_mutex_init
    if(mutex == NULL) return -1;

    //free(mutex);

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
    sched_rr(run_queue);
    
#else
    // Choose MLFQ
    sched_mlfq();
#endif
}

static void sched_rr(LinkedList *run_queue)
{
    // - your own implementation of RR
    // (feel free to modify arguments and return types)
    if(currently_running_blocked!=1 && currently_running!=NULL)
    {
        currently_running->thread_status = READY;
        insert(currently_running, currently_running->thread_id, round_robin);

    }

    if(run_queue->front!=NULL)
    {
        Node *temporary = run_queue->front;
        run_queue->front = (run_queue->front)->next;

        if(run_queue->front == NULL)
        {
            run_queue->back = NULL;
        }

        temporary->next= NULL;

        currently_running=temporary->control;
        currently_running->thread_status=RUNNING;

        currently_running_blocked=0;
        free(temporary);

        timer.it_value.tv_usec = QUANTUM;
        timer.it_value.tv_sec = 0;
        setitimer(ITIMER_PROF, &timer,NULL);

        gettimeofday(&t,NULL);
        unsigned long time = 1000000 * t.t_sec + t.t_usec;
        schedule_time[currently_running->thread_id] = time;

        setcontext(&(currently_running->value));


    }

}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq(LinkedList *run_queue)
{
    // - your own implementation of MLFQ
    // (feel free to modify arguments and return types)

    if(currently_running_blocked != 1 && currently_running != NULL) {
        currently_running->thread_status = READY;
        insert(currently_running, currently_running->thread_id, round_robin);

        int priority = currently_running->priority;

        // change the status to ready
        currently_running->thread_status = READY;

        // should we have a check for whether or not the thread yielded?
        if (yielded == 1) {
            if (priority == 4) {
                insert(round_robin, currently_running->thread_id, currently_running);
            } else if (priority == 3) {
                insert(MLFQ_level_3, currently_running->thread_id, currently_running);
            } else if (priority == 2) {
                insert(MLFQ_level_2, currently_running->thread_id, currently_running);
            } else {
                insert(MLFQ_level_1, currently_running->thread_id, currently_running);
            }

            yielded = 0;
        } else {
            if (priority == 4) {
                currently_running->priority = 3;
                insert(MLFQ_level_3, currently_running->thread_id, currently_running);
            } else if (priority == 3) {
                currently_running->priority = 2;
                insert(MLFQ_level_2, currently_running->thread_id, currently_running);
            } else if (priority == 2) {
                currently_running->priority = 1;
                insert(MLFQ_level_1, currently_running->thread_id, currently_running);
            } else {
                insert(MLFQ_level_1, currently_running->thread_id, currently_running);
            }
        }

    }

    

    return;
}

void unblock_threads() {

	//Remove each thread from blocked list and put it on run queue.
	//Free the node in blocked list as well
	Node *cur = block_list->front;
	Node *prev = block_list->back;
	
	while (cur != NULL)
	{
		cur->control->thread_status  = READY;
		#ifndef MLFQ
			// Add to the RR queue
			insert(round_robin,cur->data,cur->control);
		#else
			// int prior = cur->thrd->tcb_block-> priority;
					
			// if(prior == 1){
			// 	insert_queue(cur->thrd, mlfq_level_1);
			// }else if (prior == 2){
			// 	insert_queue(cur->thrd, mlfq_level_2);
			// }else if (prior == 3){
			// 	insert_queue(cur->thrd, mlfq_level_3);
			// }else{
			// 	insert_queue(cur->thrd, RR);
			// }	

		#endif

		cur = cur->next;
		// Free the node as the node that holds the pointer to thread is no longer
		// required in blocked list.
		free(prev);
		//Move the previosu pointer
		prev = cur;
	}

	// Make blocked list empty as all nodes are removed
	block_list->front = NULL;
	block_list->back = NULL;

}
// Feel free to add any other functions you need.
// You can also create separate files for helper functions, structures, etc.
// But make sure that the Makefile is updated to account for the same.

