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

struct TPSB {
	pthread_t tid; 
	char *tps;
};

typedef struct TPSB* tpsb_t;

int find_TID(queue_t library, void* block, void* arg)
{

        tpsb_t cur = (tpsb_t) block;
        pthread_t checkTID = (intptr_t)arg;
        if(cur->tid == checkTID)
                return 1;

        return 0;

}

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
	void* holderThing; 
        pthread_t tidHolder = pthread_self();
        queue_func_t func = &find_TID;
        int check;

	check = queue_iterate(library, func, (void*)&tidHolder, &holderThing);
        if (check == -1)
                return -1;
	queue_delete(library,(tpsb_t)holderThing);
	return 0;
}

int tps_read(size_t offset, size_t length, char *buffer)
{
	enter_critical_section();

	


	exit_critical_section();
	return 0;
}

int tps_write(size_t offset, size_t length, char *buffer)
{
	enter_critical_section();




	exit_critical_section();
	return 0;
}

int tps_clone(pthread_t tid)
{
	 enter_critical_section();

	// call tps_create() to make new TPS, copy content from target thread's TPS w/memcpy()
	void* holderThing;
	pthread_t tidHolder;
	queue_func_t func = &find_TID;
	tidHolder = pthread_self();
	int check;
	
	// if next q_iterate fails, thread with tid as argument has no TPSB or tps...
	check = queue_iterate(library, func,(void*)&tid, &holderThing);
	if (check == -1)
		return -1;
	tpsb_t mytpsb = (tpsb_t)holderThing;

	// if next q_iterate does not fail, calling thread alerady has TPSB with tps...
	check = queue_iterate(library, func, (void*)&tidHolder, &holderThing);
	if (check != -1)
		return -1;

	tps_create();
	queue_iterate(library, func, (void*)&tidHolder, &holderThing);
	tpsb_t calling_tpsb = (tpsb_t)holderThing;

	memcpy((void*)calling_tpsb->tps, (const void*)mytpsb->tps, TPS_SIZE);
	
	exit_critical_section();
	return 0;
}


