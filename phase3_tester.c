/*
 * Thread creation and yielding test
 *
 * Tests the creation of multiples threads and the fact that a parent thread
 * should get returned to before its child is executed. The way the printing,
 * thread creation and yielding is done, the program should output:
 *
 * thread1
 * thread2
 * thread3
 */


#include <stdio.h>
#include <stdlib.h>

#include "uthread.h"

int thread3(void* arg)
{
	printf("11) T3 yields, run T2 - Queue: T1 T0 T3\n");
	uthread_yield();
	printf("thread%d\n", uthread_self());
	return 0;
}

int thread2(void* arg)
{
	printf("7) T2 creates T3 and enqueue T3 - Queue: T1 T0 T3\n");
	uthread_create(thread3, NULL);
	printf("8) T2 yields, run T1 - Queue: T0 T3 T2\n");
	uthread_yield();
	printf("12) T2 prints and returns - Queue: T1 T0 T3 T2\n");
	printf("thread%d\n", uthread_self());
	return 0;
}

int thread1(void* arg)
{
	printf("4) T1 creates T2 and enqueue T2 - Queue: T0 T2\n");
	uthread_create(thread2, NULL);
	printf("5) T1 yields, attempt to run T0 - Queue: T2 T1\n");
	printf("6) T0 blocked because of join, yield, run T2 - Queue: T1 T0\n");
	uthread_yield();
	printf("9) T1 prints and yields, attempt to run T0 - Queue: T3 T2 T1\n");
	printf("thread%d\n", uthread_self());
	printf("10) T0 blocked because of join, yield, run T3 - Queue: T2 T1 T0\n");
	uthread_yield();
	printf("13) T1 returns - Queue: T0 T3 T2 T1\n");
	return 0;
}

int main(void)
{
	
	printf("1) Create main thread (T0) - Queue: empty\n");
	printf("2) create thread 1 and enqueue T1 - Queue: T1\n");
	printf("3) main yields to T1 because of join, runs T1 - Queue: T0\n");
	uthread_join(uthread_create(thread1, NULL), NULL);
	printf("14) T0 runs and isnt blocked therefore program completes\n");

	return 0;
}

