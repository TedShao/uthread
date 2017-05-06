# ECS150 Project 2 Report


## Phase 1 
  + We implemented a linked list to represent our queue, because it can be dynamically allocated and thus improves our efficiency.
  ```C
struct node{
    void        *obj;
    struct node *nxt;
};

struct queue {
    struct node*   head;
    struct node*   tail;
    int            size;
} queue;
  ```
+ Each node in the queue contains a pointer to the object as well as a pointer to the next node in the list. 
+ In `queue_delete` and `queue_iterate` we declared new objects `delete`, `link`, and `curNode` respectively. In the delete function, the `link` object is used to parse through the queue and the `delete` object is used to represent the node to delete. Similarly, in our iterate function, the `curNode` object was used to parse through the queue and represent each parsed node. 
+ By including a size attribute in our queue struct we are able to compute the length in O(1) time, since the size reflects each enqueue and dequeue made.

## Phase 2 uThread API

### Data Structure Design
+ Here is the TCB data structure implemented:
 ```C
  struct tcb{
	uthread_t	tid;	// tid
	uthread_ctx_t	ctx;	// context of the thread: set of registers
	void*	sPtr;		// pointer to stack
	state	stat; 		// READY, RUNNING, ZOMBIE, EXIT
	void*	returnVal;	// return value when exit
	void*   argVal;		// holds argument
	int	blocks		// number of children blocking thread
	uthread_t	parent;	// parent tid
};

// global queue 
queue_t uthread_queue;

// thread assign counter
uthread_t assign;

// (global) pointer to the current thread
struct tcb* current;
struct tcb* next;
 ```
 + Our TCB structure reflects the necessary information as specified on the assignment (tid, context, stack pointer, status) with added attribute to make implementation more efficient
+ `returnVal` holds the return value, if any, for the respective thread
+ `parent` holds the TID of it's parent thread. This allows the program to let the parent know when it has finished executing.
+ `blocks` holds the total number of children blocking the respective thread. This allows the program to determine when the respective thread is in the READY state again.

### Global Variables
+ `uthread_queue` : our implementation consists of one main, global queue.
+ `assign` : this serves as a TID assign counter that assigns each thread a unique TID.
+ `current` : a TCB object pointer that points to the current running thread. This allows for an efficient implementation since at any given moment of the process, the program is aware of its current running process. 
+ `next` : a TCB object pointer that points to the next available thread that is ready. This makes scheduling more efficient, since the program will always know which thread is ready to begin processing next.
 
 ### Functions

**uthread_initial**
+ In our intialize function, we create `uthread_queue`, as well as a new TCB object to represent our main thread. 
The mainThread receives a TID of 0 and is set to the RUNNING state. Additionally, we set our glocal TCB object to the mainThread.
This function is only called once during main and is different from `uthread_create`.

**uthread_create**
+ We define a new TCB object by assigning a new tid, as well creating a new context, then enqueueing it to the global queue, ready for processing. if it is the first time being called, we assume that the current process is the main process and call function initial. 

**uthread_exit**
+ We only exit the thread if the current running thread's status is set to READY. If new thread is ready, we set our current thread
to the ZOMBIE state and change the state of the new thread to RUNNING. 

**uthread_yield**
+ The while loop traverse through the global queue uthread_queue to see if the oldest item in the queue is in the READY state. 
Once a new thread is found READY, we update the state of the current running thread with that of the new thread to be run. 
The old running process is enqueued to the back of the queue and their contexts are saved and switched.
+ We implemented yield by calling our helper scheduling program, scheduleNext(), updating the respective statuses, and switching context from next ,the next available thread, and current, the current running thread.

**scheduleNext**
+ This helper function looks for the next the available ready thread in the queue. It also updates (unblocks) threads if the respective thread no longer has an children blocking it, by checking the block count as it looks for the next available thread.

**uthread_self**
+ As stated in the struct definition, uthread_self() is implemented by calling to the global variable current and returning its tid.

**uthread_exit**
+ For exit, we implemented a helper program, dec_blocks, that allows the function to update the block count on the queue for each respective queue (tcb) object. Then get the next available thread, updating the respective status, and switch context. We enqueue the zombie thread to allow for its parent to pick up later.

**dec_blocks**
+ A helper function that is passed in to queue_iterate, in order to update the block count across all queue elements.

**uthread_join**
+ In phase 2, we implemented a loop that breaks only when the queue is empty and continues to switch to the next available thread.


## Phase 3 uthread_join() *(incomplete)*
**find_status**
+ This is a helper function to find the child passed in the parameter in uthread_join(), implemented by calling the queue_iterate function. 

**scheduleNext**
+ This helper function looks for the next the available ready thread in the queue. It also updates (unblocks) threads if the respective thread no longer has an children blocking it, by checking the block count as it looks for the next available thread.

**exit()**
+ For exit, we implemented a helper program, dec_blocks, that allows the function to update the block count on the queue for each respective queue (tcb) object. Then get the next available thread, updating the respective status, and switch context. We enqueue the zombie thread to allow for its parent to pick up later.

**dec_blocks**
+ This is a helper function that is passed in to queue_iterate, in order to update the block count across all queue elements.

**implementation idea:**
+ Using all of the new helper functions and data structure changes, we planned to make the process of keeping in account the number of blocked threads and scheduling the next thread easier to maintain. Thereby, allowing the process of joining threads easier. 

### Functions

**find_status**
+ A helper function to find the child passed in the parameter in uthread_join(), implemented by calling the queue_iterate function. 

## Phase 4 Preemption
  

  
  
  ### Sources 
  We used [stackoverflow.com](https://www.stackoverflow.com) as a resource for debugging issues only. 
