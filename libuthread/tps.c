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

int getTPS(void* tpsHolder, pthread_t tid)
{
        pthread_t tidHolder = pthread_self();
        queue_func_t func = &find_TID;
        if (tid == NULL)
	{
		int check = queue_iterate(library, func, (void*)&tidHolder, &tpsHolder);
		if (check == -1)
			return -1;
	}
	int check = queue_iterate(library, func, (void*)&tid, &tpsHolder);	
	if (check == -1)
		return -1;
 
	tpsb_t myTps = (tpsb_t)tpsHolder;
	if (myTps->tid != tidHolder)
		return -1;
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
	void* tpsHolder;
	int check = getTPS(tpsHolder, NULL);
        if (check == -1)
                return -1;

	queue_delete(library,(tpsb_t)tpsHolder);
	return 0;
}

int tps_read(size_t offset, size_t length, char *buffer)
{
	enter_critical_section();
	void* tpsHolder;
        int check = getTPS(tpsHolder, NULL);
        if (check == -1 || buffer == NULL || offset + length > TPS_SIZE)
                return -1;
	tpsb_t calling_tpsb = (tpsb_t)tpsHolder;
	
	memcpy(buffer, calling_tpsb->tps + offset,length);
	exit_critical_section();
	return 0;
}

int tps_write(size_t offset, size_t length, char *buffer)
{
	enter_critical_section();
	void* tpsHolder;
        int check = getTPS(tpsHolder, NULL);
        if (check == -1 || buffer == NULL || offset + length > TPS_SIZE)
                return -1;
	tpsb_t calling_tpsb = (tpsb_t)tpsHolder;
        
	memcpy(calling_tpsb->tps + offset, buffer, length);
	exit_critical_section();
	return 0;
}

int tps_clone(pthread_t tid)
{
	 enter_critical_section();

	// call tps_create() to make new TPS, copy content from target thread's TPS w/memcpy()
	void* toBeClnd;
	void* wantsToCln;
	int check;
	
	// if next q_iterate fails, thread with tid as argument has no TPSB or tps...
	check = getTPS(toBeClnd, tid);
	if (check == -1)
		return -1;
	tpsb_t toBeClndCstd = (tpsb_t)toBeClnd;

	// if next q_iterate does not fail, calling thread alerady has TPSB with tps...
	check = getTPS(wantsToCln, NULL);
	if (check != -1)
		return -1;

	tps_create();
	check = getTPS(wantsToCln, NULL);
	if (check == -1)
		return -1;
	tpsb_t wantsToClnCstd = (tpsb_t)wantsToCln;

	memcpy((void*)wantsToClnCstd->tps, (const void*)toBeClndCstd->tps, TPS_SIZE*sizeof(char));
	
	exit_critical_section();
	return 0;
}


