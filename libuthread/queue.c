#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>


#include "queue.h"

struct node{
    void        *obj;
    struct node *nxt;
};

struct queue {
    struct node*   head;
    struct node*   tail;
    int            size;
} queue;



/*
 * queue_create - Allocate an empty queue
 *
 * Create a new object of type 'struct queue' and return its address.
 *
 * Return: Pointer to new empty queue. NULL in case of failure when allocating
 * the new queue.
 */
queue_t queue_create(void)
{
    // create object of type queue_t
	queue_t newqueue;

    // allocate space
	newqueue = (struct queue *)malloc(sizeof(struct queue));

    // set head and tail pointers 
	newqueue -> head = newqueue -> tail = NULL;

    // set size 
    newqueue -> size = 0;

	return newqueue;
}



/*
 * queue_destroy - Deallocate a queue
 * @queue: Queue to deallocate
 *
 * Deallocate the memory associated to the queue object pointed by @queue.
 *
 * Return: -1 if @queue is NULL of if @queue is not empty. 0 if @queue was
 * successfully destroyed.
 */
int queue_destroy(queue_t queue)
{
    // check for correct parameters
    if(queue == NULL || (queue -> head != NULL && queue -> tail != NULL))
        return -1;
    else
        free(queue);

    /*while(queue -> head != NULL)
    {
        // save value in dummy node
        struct node* delete = (struct node*)malloc(sizeof(struct node));

        // call dequeue and free dummy node
        queue_dequeue(queue, delete->obj);
        free(delete);
    }*/

	return 0;
}



 /*
 * queue_enqueue - Enqueue data item
 * @queue: Queue in which to enqueue item
 * @data: Address of data item to enqueue
 *
 * Enqueue the address contained in @data in the queue @queue.
 *
 * Return: -1 if @queue or @data are NULL, or in case of memory allocation error
 * when enqueing. 0 if @data was successfully enqueued in @queue.
 */
int queue_enqueue(queue_t queue, void *data)
{
    // check for correct parameters
    if(data == NULL || queue == NULL)
        return -1;

    // allocate space for new node object
    struct node* add = (struct node*) malloc(sizeof(struct node));

    // define parameters for object
    add -> obj = data;
    add -> nxt = NULL;

    // if the queue is empty
    if (queue -> head == NULL && queue -> tail == NULL)
    {
        queue -> head = queue -> tail = add;
        (queue -> size)++;

        return 0;
    }

    // (else) add to end of queue
    queue -> tail -> nxt = add;
    queue -> tail = add;
    (queue -> size)++;

	return 0;
}



/*
 * queue_dequeue - Dequeue data item
 * @queue: Queue in which to dequeue item
 * @data: Address of data pointer where item is received
 *
 * Remove the oldest item of queue @queue and assign this item (the value of a
 * pointer) to @data.
 *
 * Return: -1 if @queue or @data are NULL, or if the queue is empty. 0 if @data
 * was set with the oldest item available in @queue.
 */
int queue_dequeue(queue_t queue, void **data)
{
    // check for correct parameters
    if(queue == NULL)
        return -1;

    // assign object value of the pointer to data 
    struct node* delete = (struct node*) malloc(sizeof(struct node));
    delete = (queue -> head -> obj);
   
    *data = &delete -> obj;

    if(queue -> head == queue -> tail) // if only one object in queue
    {
        // set the queue to NULL
        queue -> head = queue -> tail = NULL;
        (queue -> size) = 0;
    }
    else
    {
        // set head to the next available obj
        queue -> head = queue -> head -> nxt;
        (queue -> size)--;
    }
    
	return 0;
}



/*
 * queue_delete - Delete data item
 * @queue: Queue in which to delete item
 * @data: Data to delete
 *
 * Find in queue @queue, the first (ie oldest) item equal to @data and delete
 * this item.
 *
 * Return: -1 if @queue or @data are NULL, of if @data was not found in the
 * queue. 0 if @data was found and deleted from @queue.
 */
int queue_delete(queue_t queue, void *data)
{
    // check for correct parameters
    if(queue == NULL || data == NULL)
        return -1;


    if( queue -> head -> obj == data) // if data is at head of node
    {
        struct node* delete = (struct node*) malloc(sizeof(struct node));
        queue_dequeue(queue, delete->obj);
        free(delete);
    }
    else // if data is not at head of node
    {
        // allocate memory for @delete (node to delete) & @link (rest of the queue after data)
        struct node* delete = (struct node*) malloc(sizeof(struct node));
        struct node* link = (struct node*) malloc(sizeof(struct node));

        // use link to parse queue
        link = queue -> head;

        while( link -> nxt != NULL && link -> nxt -> obj != data)
            link = link -> nxt;

        // if data not found return -1
        if(link -> nxt == NULL)
            return -1;

        // set delete to node with data to free
        delete = link -> nxt;
        free(delete);
        (queue -> size)--;

        // relink the queue with the rest of the queue
        link -> nxt = link -> nxt -> nxt;
        queue_enqueue(queue, link);
        (queue -> size)--;

        free(link);
    }


    // queue output test for debugging 
    /* printf("  print test :  \n");
    
    struct node* print = (struct node*) malloc(sizeof(struct node));

    print = queue -> head;
    int size = 0;

    while(print -> nxt != NULL)
    {
        printf("  [%d] = %p \n", size, print -> obj);

        print = print -> nxt;
        size ++;
    }*/

	return 0;
}

/*
 * queue_iterate - Iterate on all items of a queue
 * @queue: Queue to iterate on
 * @func: Function to call on each queue item
 * @arg: Extra argument to be given to the callback function
 * @data: Address of data pointer where possible item is received
 *
 * This function iterates on every item of the queue @queue and calls the given
 * callback function @func. When calling @func, pass the queue being iterated
 * over, the current data item and the extra argument @arg.
 *
 * If @func returns 1 for a specific item, the iteration stops. If @data is
 * different than NULL, then it receives the data item where the iteration has
 * stopped.
 *
 * We assume that queue_delete() cannot be called inside @func on the current
 * item. Doing so would result in an undefined behavior.
 *
 * Return: -1 if @queue or @func are NULL. 0 if @queue was iterated
 * successfully.
 */
int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
    // check for correct parameters
    if(queue == NULL || func == NULL )
        return -1;

    // use curNode as a parser for the queue
    struct node* curNode = (struct node*) malloc(sizeof(struct node));
    curNode = queue -> head;

    for(int i = 0; i < queue->size ; i++)
    {
        // if func call returns 1 - deal with saving data
        if( func(queue, curNode -> obj, arg) == 1) 
        {
            if(data != NULL)
                *data = (curNode -> obj);

            break;
        }
        else
            curNode = curNode -> nxt;
    }

    free(curNode);
	return 0;
}


/*
 * queue_length - Queue length
 * @queue: Queue to get the length of
 *
 * Return the length of queue @queue.
 *
 * Return: -1 if @queue is NULL. Length of @queue otherwise.
 */
int queue_length(queue_t queue)
{ 
    if(queue -> head == NULL && queue -> tail == NULL)
	   return -1;
    else
       return (queue -> size);
}
