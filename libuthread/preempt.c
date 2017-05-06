#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>


#include "preempt.h"
#include "uthread.h"


void timer_handler (int signum)
{
	// force current thread to yield
	printf("forcing yield... \n");
	uthread_yield();

 	//static int count = 0;
 	//printf ("timer expired %d times\n", ++count);

}

void preempt_start(void)
{


	 struct sigaction sa;
	 struct itimerval timer;

	 // Install timer_handler as the signal handler for SIGVTALRM. 
	 memset (&sa, 0, sizeof (sa));
	 sa.sa_handler = &timer_handler; // pointer to function
	 sigaction (SIGVTALRM, &sa, NULL);



	 // Configure the timer to expire after 100 Hz = 10,000 msec... 
	 timer.it_value.tv_sec = 0;
	 timer.it_value.tv_usec = 10000;

	 // and in 10,000 ms intervals
	 timer.it_interval.tv_sec = 0;
	 timer.it_interval.tv_usec = 10000;
	 
	 setitimer (ITIMER_VIRTUAL, &timer, NULL);


}

void preempt_enable(void)
{


	// block signals of type SIGVTALARM
	 sigset_t interrupt_alarm;

	 // initialize signal mask
	sigemptyset (&interrupt_alarm);
  	sigaddset (&interrupt_alarm, SIGVTALRM);

	sigprocmask (SIG_UNBLOCK, &interrupt_alarm, NULL);

	//pthread_sigmask(SIG_UNBLOCK, &interrupt_alarm, NULL);
          

}

void preempt_disable(void)
{
	// block signals of type SIGVTALARM
	 sigset_t interrupt_alarm;

	 // initialize signal mask
	sigemptyset (&interrupt_alarm);
  	sigaddset (&interrupt_alarm, SIGVTALRM);

  	sigprocmask (SIG_BLOCK, &interrupt_alarm, NULL);

  	//pthread_sigmask(SIG_BLOCK, &interrupt_alarm, NULL);
         


}









