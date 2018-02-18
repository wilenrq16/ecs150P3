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
	mySem->queue = queue_create();
	if (mySem == NULL || mySem->queue == NULL)
		return NULL;
	return mySem;   
}

int sem_destroy(sem_t sem)
{
	if (sem == NULL || queue_length(sem->queue) != 0)
		return -1;
	queue_destroy(sem->queue); 
	free(sem);
	return 0; 
}

int sem_down(sem_t sem)
{
	enter_critical_section();
	if (sem == NULL)
		return -1;
	while (sem->value == 0)
	{
	pthread_t tid;
	tid = pthread_self();
	queue_enqueue(sem->queue, &tid);	
	thread_block(); 	
	}	
	if (sem->value == 1)
		sem->value -= 1;
	// Take resource from sem??
	exit_critical_section();
	return 0;
}

int sem_up(sem_t sem)
{
	enter_critical_section();
	if (sem == NULL)
		return -1;
	sem->value += 1;
	if (queue_length(sem->queue) != 0)
	{
		void* holderThing;
		queue_dequeue(sem->queue, &holderThing); 
		thread_unblock(*((pthread_t*)holderThing));
	}
	// Release resource, give to sem??	
	exit_critical_section();
	return 0; 
}

