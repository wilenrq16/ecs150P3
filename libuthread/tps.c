#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "queue.h"
#include "thread.h"
#include "tps.h"

queue_t library = NULL;
#define OFFSET 0
#define FD -1

typedef struct TPSB {
	pthread_t tid; 
	void* tps;
} TPSB;

typedef struct TPSB* tpsb_t;

int tps_init(int segv)
{
	if (library == NULL)
	{
	// Maybe make a queue to hold TPS objects? 
	queue_t queue = queue_create();
	if (queue == NULL)
		return -1;
	library = queue; 
	return 0; 
	}
	return -1;
}

int tps_create(void)
{
	// Maybe make TPS object, associate to thread with TID, and store in queue?
	// Use mmap(); memory page needs to be private, anonymous, accessible in read& writes
 	tpsb_t tpsb = (tpsb_t)malloc(sizeof(struct TPSB));
	tpsb->tid = pthread_self();
	tpsb->tps = mmap(NULL, TPS_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, FD, OFFSET); 
	if (tpsb->tps == MAP_FAILED)
		return -1;
	queue_enqueue(library, tpsb);
	return 0; 
}

int tps_destroy(void)
{
	return 0;
}

int tps_read(size_t offset, size_t length, char *buffer)
{
	return 0;
}

int tps_write(size_t offset, size_t length, char *buffer)
{
	return 0;
}

int tps_clone(pthread_t tid)
{
	// call tps_create() to make new TPS, copy content from target thread's TPS w/memcpy()
	return 0;
}

