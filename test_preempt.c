#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>


#include "preempt.h"
#include "uthread.h"

int hello(void* arg)
{
	printf("Hello world!\n");

	printf(" currently in tid %d \n", uthread_self());

	return 0;
}

int main(void)
{

	uthread_t tid1, tid2;

	tid1 = uthread_create(hello, NULL);
	tid2 = uthread_create(hello, NULL);

	printf("tid1 = %d \n", tid1);
	printf("tid2 = %d \n", tid2);


	// start thread preemption
	preempt_start();

	preempt_disable();

	preempt_enable();

	return 0;


}