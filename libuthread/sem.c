#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "thread.h"

struct semaphore {
	size_t value;
	queue_t queue;  
};

sem_t sem_create(size_t count)
{
	sem_t mySem = (sem_t)malloc(sizeof(struct semaphore));
	mySem->value = count; 
}

int sem_destroy(sem_t sem)
{
	/* TODO: Phase 1 */
}

int sem_down(sem_t sem)
{
	if (sem == 0)
		return -1;
	while (sem->value == 0)
	pthread_t* tid;
	tid = pthread_self();
	queue_enqueue(sem->queue, tid);	
	thread_block(); 	
		
	if (sem->value == 1)
		sem->value -= 1;
	
	return 0;
}

int sem_up(sem_t sem)
{
	if (sem == 0)
		return -1;
	sem->value += 1;
	void** holderThing;
	queue_dequeue(sem->queue, holderThing);
	thread_unblock(**holderThing);
		
	
	return 0; 
}

