CC = gcc
CCFLAGS = -Wall -Werror -g

libuthread.a: context.o preempt.o queue.o uthread.o
	ar rcs libuthread.a context.o preempt.o queue.o uthread.o

preempt.o: preempt.c preempt.h
	$(CC) $(CCFLAGS) -c preempt.c

context.o: context.c context.h
	$(CC) $(CCFLAGS) -c context.c

uthread.o: uthread.c uthread.h
	$(CC) $(CCFLAGS) -c uthread.c

queue.o: queue.c queue.h
	$(CC) $(CCFLAGS) -c queue.c


clean:
	rm -f libuthread.a queue.o context.o uthread.o preempt.o 
