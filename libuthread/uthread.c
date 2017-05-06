#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#include "context.h"
#include "preempt.h"
#include "queue.h"
#include "uthread.h"

#define USHRT_MAX 32768

/* finished 

	PHASE 2 

	1) uthread_create() --------------------------------------------------------
		a) first time being called - 
			(initialization function for MAIN thread)
			initialize uthread library by registering the so-far single execution 
			flow of the application as the main thread that the library can 
			schedule as a regular thread
		b) can be called to create more threads - 
			(initialization for threads in general)

	2) uthread_yield() ---------------------------------------------------------
		(called to ask library's scheduler to pick and run the next 
		available thread)
		** non preemptive mode -> non-compliant thread that never yields can 
									keep the processing resource for itself.

	3) uthread_join() ----------------------------------------------------------
		(assume only called by main)
		execute infinite loop:
			a) if no more threads ready to run in system -> break loop, return
			b) else -> yield to next available thread

*/


// thread states
//	when active - running, ready, or blocked
//  when inactive - zombie
typedef enum {READY, RUNNING, BLOCKED, ZOMBIE} state; 

// TCB struct
struct tcb{
	uthread_t 		tid; 		// tid
	uthread_ctx_t	ctx; 		// context of the thread: set of registers
	void*			sPtr; 		// pointer to stack
	state			stat; 		// READY, RUNNING, ZOMBIE, EXIT
	void* 			returnVal;	// return value when exit
	void*			argVal;		// holds argument 
	int 			blocks;		// number of children blocking thread
	uthread_t 		parent;		// parent tid
};

// global queue 
queue_t uthread_queue;

// thread assign counter
uthread_t assign;

// (global) pointer to the current thread
struct tcb* current;

// pointer to the next "READY" thread
struct tcb* next;

/* FUNCTIONS NOT USED TIL PHASE 3

// function to check the status of a given tid
static int find_status(queue_t q, void *data, void *arg)
{	
	struct tcb* thread = data;
	uthread_t match = (uthread_t)(intptr_t)arg;

    if((thread->tid) == match)
    	return 1;
    
    return 0;
}


// function to look for threads who were blocked by the finished thread and decrememnts blocks count
static int dec_blocks(queue_t q, void *data, void *arg)
{	
	struct tcb* thread = data;
	uthread_t match = (uthread_t)(intptr_t)arg;

    if((thread->parent) == match)
		thread->blocks--;
    
    return 0;
}
*/


// uthread_initial - Initialize global variables 
void uthread_initial(void)
{
	// initialize thread assign 
	assign = 0;

	// allocated space for queue
	uthread_queue = malloc(sizeof(uthread_queue));

	// initalize queue 
	uthread_queue = queue_create();

	// initalize a tcb for main thread and new thread
	struct tcb* mainThread = (struct tcb*)malloc(sizeof(struct tcb));

	uthread_ctx_t ctxNull;
		
	// initialize main tcb
	mainThread->tid = (short)0;
	mainThread->ctx = ctxNull;
	mainThread->sPtr = (void*)NULL;
	mainThread->stat = RUNNING;
	mainThread->parent = -1;

	// assign current Thread pointer
	current = (struct tcb*)malloc(sizeof(struct tcb));
	current = mainThread;

	// initialize global pointer to next thread
	next = (struct tcb*)malloc(sizeof(struct tcb));
}

/*
 * uthread_yield - Yield execution
 *
 * This function is to be called from the currently active and running thread in
 * order to yield for other threads to execute.
 */
void uthread_yield(void)
{
	// first tcb object on queue
	struct tcb* newThread = malloc(sizeof(struct tcb));
	struct tcb* curThread = malloc(sizeof(struct tcb));

	queue_dequeue(uthread_queue, (void**)&newThread);

	while(newThread->stat != READY)
	{
		queue_enqueue(uthread_queue, newThread);
		queue_dequeue(uthread_queue, (void**)&newThread);
	}

	// check if new thread is ready
	if( newThread->stat == READY )
	{
		curThread = current;

		// update state of threads

		curThread->stat = READY;
		newThread->stat = RUNNING;

		// get old context
		getcontext(&(curThread->ctx));

		queue_enqueue(uthread_queue, (void*)curThread);

		// reset current context
		current = newThread;

		// switch from current to new thread
		uthread_ctx_switch(&(curThread->ctx), &(newThread->ctx));
	}
}


/*
 * uthread_exit - Exit from currently running thread
 * @retval: Return value
 *
 * This function is to be called from the currently active and running thread in
 * order to finish its execution. The return value @retval is to be collected
 * from a joining thread.
 *
 * A thread which has not been 'collected' should stay in a zombie state. This
 * means that until collection, the resources associated to a zombie thread
 * should not be freed.
 *
 * This function shall never return.
 */
void uthread_exit(int retval)
{
	// first tcb object on queue
	struct tcb* newThread = malloc(sizeof(struct tcb));
	struct tcb* curThread = malloc(sizeof(struct tcb));

	queue_dequeue(uthread_queue, (void**)&newThread);

	// check if new thread is ready
	if( newThread->stat == READY )
	{
		curThread = current;

		// update state of threads
		curThread->stat = ZOMBIE;
		newThread->stat = RUNNING;

		// get old context
		getcontext(&(curThread->ctx));

		curThread->returnVal = &retval;

		queue_enqueue(uthread_queue, (void*)newThread);

		// reset current context
		current = newThread;

		// switch from current to new thread
		uthread_ctx_switch(&(curThread->ctx), &(newThread->ctx));
	}
}  

/*
 * uthread_create - Create a new thread
 * @func: Function to be executed by the thread
 * @arg: Argument to be passed to the thread
 *
 * This function creates a new thread running the function @func to which
 * argument @arg is passed, and returns the TID of this new thread.
 *
 * Return: -1 in case of failure (memory allocation, context creation, TID
 * overflow, etc.). The TID of the new thread otherwise.
*/
int uthread_create(uthread_func_t func, void *arg)
{
	// call uthread_initial if global variables have not been set up
	if (uthread_queue == NULL)
		uthread_initial();

	// allocate space for new thread
	struct tcb* newThread = malloc(sizeof(struct tcb));

	// set parent pid
	newThread -> parent = current -> tid;

	// initializing blocks count
	newThread -> blocks = 0;

	// initializing argument value
	newThread -> argVal = arg;

	// tid
	(short)assign++;
	newThread->tid = (short)assign;

	// allocate new stack 
	newThread->sPtr = uthread_ctx_alloc_stack();

	// if failure 
	if( uthread_ctx_init(&(newThread->ctx), newThread->sPtr, func, newThread -> argVal) == -1 || newThread->sPtr == NULL || newThread->tid > USHRT_MAX)
		return -1;

	// states
	newThread->stat = READY;

	// enqueue new thread object on global queue
	queue_enqueue(uthread_queue, (void*)newThread);
	
	return newThread->tid;
}

/*
 * uthread_self - Get thread identifier
 *
 * Return: The TID of the currently running thread
 */
uthread_t uthread_self(void)
{
	// call pointer to current tcd object
	return current->tid;
}

int uthread_join(uthread_t tid, int *retval)
{
	//PHASE 2 IMPLEMENTATION
	int count = 0;
	while(uthread_queue != NULL)
	{
		if(queue_length(uthread_queue) == -1)
			break;
		else
		{
			uthread_yield();
		}
		count++;
	}
	return count;
}


/* PHASE 3 ATTEMPT -- 


	PHASE 3

	uthread_join()

		when a thread(T1) joins another thread(T2):
		(a) if T2 is active, T1 must be blocked until T2 dies -> T1 is unblocks and collects T2
		(b) T2 is already dead, T1 can collect T2 right away

		** once T2 is collect --> resource can be freed

// function that gets the next scheduled thread
void scheduleNext(void)
{
	struct tcb* nextThread = malloc(sizeof(nextThread));
	printf(" .. current size of queue is %d \n", queue_length(uthread_queue));

	printf("   ********** currently in scheduleNext()\n");

	printf("status of dequeue = %d\n", queue_dequeue(uthread_queue, (void**)&nextThread));

	if( nextThread == NULL)
		printf("     next thread is empty? \n");
	else
		printf("	but thread is %d \n", queue_length(uthread_queue));

	while(nextThread != NULL)
	{
		printf("     test for thread %d ?\n", nextThread->tid);
		if (nextThread->stat == READY )
			printf("     currently thread %d is ready\n", nextThread->tid);
		else if (nextThread->stat == RUNNING )
			printf("     currently thread %d is running\n", nextThread->tid);
		else if (nextThread->stat == BLOCKED )
			printf("     currently thread %d is blocked\n", nextThread->tid);
		else if (nextThread->stat == ZOMBIE )
			printf("     currently thread %d is zombie\n", nextThread->tid);


		// check if can BLOCKED -> READY
		if(nextThread->blocks == 0 && nextThread->stat == BLOCKED)
		{
			printf("     test for thread %d unblock?\n", nextThread->tid);
			nextThread->stat = READY;
			next = nextThread;
			return;
		}
		else if(nextThread->stat == READY)
		{
			printf("     test for thread %d ready?\n", nextThread->tid);
			next = nextThread;
			return;
		}
		else
		{
			printf("     next one\n");
			queue_enqueue(uthread_queue, (void*)nextThread);
			queue_dequeue(uthread_queue, (void**)&nextThread);
		}
	}

	printf("   thread %d passed \n", nextThread->tid);
}

void uthread_yield(void)
{
	// first tcb object on queue
	struct tcb* newThread = malloc(sizeof(struct tcb));
	struct tcb* curThread = malloc(sizeof(struct tcb));

	if(next->stat != READY || next == NULL)
		scheduleNext();

	newThread = next;

	curThread = current;

	// update state of threads

	curThread->stat = READY;
	newThread->stat = RUNNING;

	// get old context
	getcontext(&(curThread->ctx));

	queue_enqueue(uthread_queue, (void*)curThread);

	// reset current context
	current = newThread;

	// switch from current to new thread
	uthread_ctx_switch(&(curThread->ctx), &(newThread->ctx));
	
}

void uthread_exit(int retval)
{
	// ***iterate function to decrement count for blocks when child exits
	queue_iterate(uthread_queue, dec_blocks, (void*)(intptr_t)uthread_self(), (void **)NULL);

	// first tcb object on queue
	struct tcb* newThread = malloc(sizeof(struct tcb));
	struct tcb* curThread = malloc(sizeof(struct tcb));

	if(next->stat != READY || next == NULL)
		scheduleNext();

	newThread = next;

	curThread = current;

	// update state of threads
	curThread->stat = ZOMBIE;
	newThread->stat = RUNNING;

	// get old context
	getcontext(&(curThread->ctx));

	curThread-> returnVal = &retval;

	queue_enqueue(uthread_queue, (void*)newThread);

	// reset current context
	current = newThread;

	// switch from current to new thread
	uthread_ctx_switch(&(curThread->ctx), &(newThread->ctx));

}



int uthread_join(uthread_t tid, int *retval)
{
	//PHASE 3 IMPLEMENTATION

	//	when parent joins with child

	//	** check that child has exited : (queue_iterate for status of child)

	//		if yes:
	//			free child -> keep running parent
	//		if not:
	//			block P and dont schedule until child has exited
	//			load the next thread


	printf("in join ***************\n");
	printf(" .. current size of queue is %d \n", queue_length(uthread_queue));
	struct tcb* child = malloc(sizeof(struct tcb));
	struct tcb* curThread = malloc(sizeof(struct tcb));
	struct tcb* newThread = malloc(sizeof(struct tcb));
	printf(" .. current running process is is %d \n", uthread_self());
	queue_iterate(uthread_queue, find_status, (void*)(intptr_t) tid, (void **)&child);
	printf(" .. current size of queue is %d \n", queue_length(uthread_queue));
	printf(" .. got tid of child = %d \n", child->tid);
	printf(" .. current size of queue is %d \n", queue_length(uthread_queue));

	
	// tester for using queue_iterate -> find_status 
		printf("currently in thread %d but found child %d \n", uthread_self(), child->tid);
	

	scheduleNext();

	printf("currently here** \n");
	if (child == NULL || child->parent != uthread_self())
		return -1;
	else if (child->stat != ZOMBIE)
	{
		printf("currently here*** \n");
		// block parent until child has exited
		curThread = current;
		curThread->stat = BLOCKED;
		curThread->blocks++;
		queue_enqueue(uthread_queue, curThread);
		printf(" .. just enqueued %d \n", curThread->tid);

		newThread = next;

		printf(" .. next schedule thread is %d \n", newThread->tid);

		newThread->stat = RUNNING;

		getcontext(&(curThread->ctx));

		current = newThread;

		uthread_ctx_switch(&(curThread->ctx), &(newThread->ctx));
	}
	else if (child->stat == ZOMBIE) // if child has exited - free 
	{
		// collecting child 
		current->argVal = child->returnVal;
		queue_delete(uthread_queue, child); 
	}
	

	printf("currently here**** \n");

	return 0;
}


*/







