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
#define NA -2 //"Not Applicable"

/* Struct to house a thread's tid and TPS Page. 
Pointers to this type are library stored (enqueued). */
struct TPSB {
	pthread_t tid; 
	char *tps;
};

typedef struct TPSB* tpsb_t;

int find_TID(queue_t library, void* block, void* arg)
{

        tpsb_t cur = (tpsb_t)block;
        pthread_t checkTID = (intptr_t)arg;
        if(cur->tid == checkTID)
                return 1;

        return 0;
}

int find_TPS(queue_t library, void* block, void* arg)
{
	tpsb_t cur = (tpsb_t)block;
	char* checkTPS = (char*)arg;
	if (cur->tps == checkTPS)
		return -1;
	return 0;
}

int getTPS(void* tpsHolder, pthread_t tid)
{
        pthread_t tidHolder = pthread_self();
        queue_func_t func = &find_TID;
        if (tid == NA)
	{
		int check = queue_iterate(library, func, (void*)&tidHolder, &tpsHolder);
		if (check == -1)
			return -1;
	}
	int check = queue_iterate(library, func, (void*)&tid, &tpsHolder);	
	if (check == -1)
		return -1;
 
	tpsb_t myTps = (tpsb_t)tpsHolder;
	if (myTps->tid != tidHolder) // SEG FAULT OCCURS HERE
		return -1;
	return 0; 
}

void segv_handler(int sig, siginfo_t *si, void *context)
{
	/*
	* Get the address corresponding to the beginning of the page where the
	* fault occurred
	*/
	void* tpsHolder;
	void* p_fault = (void*)((uintptr_t)si->si_addr & ~(TPS_SIZE - 1));

	queue_func_t func = &find_TPS;	    
	queue_iterate(library, func, p_fault, &tpsHolder);
	tpsb_t potentialM = (tpsb_t)tpsHolder;
 
	if (potentialM->tps == (char*)p_fault)
        /* Printf the following error message */
        fprintf(stderr, "TPS protection error!\n");

	/* In any case, restore the default signal handlers */
	signal(SIGSEGV, SIG_DFL);
	signal(SIGBUS, SIG_DFL);
	/* And transmit the signal again in order to cause the program to crash */
	raise(sig);
}

int tps_init(int segv)
{
	if (library == NULL)
	{
		if (segv) 
		{
        		struct sigaction sa;

        		sigemptyset(&sa.sa_mask);
        		sa.sa_flags = SA_SIGINFO;
        		sa.sa_sigaction = segv_handler;
        		sigaction(SIGBUS, &sa, NULL);
        		sigaction(SIGSEGV, &sa, NULL);
    		}
		// Creatse a queue library to hold tpsb_t objects 
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
	// Create tps_b object, associate to thread with TID, and store in queue?
	// Memory page is set to private, anonymous, and w/o read/write permissions
 	tpsb_t tpsb = (tpsb_t)malloc(sizeof(struct TPSB));
	tpsb->tid = pthread_self();
	tpsb->tps = mmap(NULL, TPS_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, FD, OFFSET); 
	if (tpsb->tps == MAP_FAILED)
		return -1;
	queue_enqueue(library, tpsb);
	return 0; 
}

int tps_destroy(void)
{	
	enter_critical_section();
	void* tpsHolder = NULL;
	int check = getTPS(tpsHolder, NA);
        if (check == -1)
                return -1;
	tpsb_t toDel = (tpsb_t)tpsHolder;
	free(toDel->tps);
	queue_delete(library, toDel);
	exit_critical_section();
	return 0;
}

int tps_read(size_t offset, size_t length, char *buffer)
{
	enter_critical_section();	
	void* tpsHolder = NULL;
        int check = getTPS(tpsHolder, NA);
	// Check to see if getTPS fails, buffer is null, or if read amount exceeds page
        if (check == -1 || buffer == NULL || offset + length >= TPS_SIZE)
                return -1;
	tpsb_t calling_tpsb = (tpsb_t)tpsHolder;
	// Change read permissions
        check =  mprotect((void*)&calling_tpsb->tps[offset], length, PROT_READ);
	if (check == -1)
		return -1; 
	memcpy((void*)buffer, (const void*)&calling_tpsb->tps[offset], length);
	// Change permissions back 
	check = mprotect((void*)&calling_tpsb->tps[offset], length, PROT_NONE);
	if (check == -1)
		return -1; 
	exit_critical_section();
	return 0;
}

int tps_write(size_t offset, size_t length, char *buffer)
{
	enter_critical_section();
	void* tpsHolder = NULL;
        int check = getTPS(tpsHolder, NA);
	// Check to see if getTPS fails, buffer is null, or if write amount exceeds page
        if (check == -1 || buffer == NULL || offset + length >= TPS_SIZE)
                return -1;
	tpsb_t calling_tpsb = (tpsb_t)tpsHolder;
        // Change write permissions
	check = mprotect((void*)&calling_tpsb->tps[offset], length, PROT_WRITE);
	if (check == -1)
		return -1;
	memcpy((void*)&calling_tpsb->tps[offset], (const void*)buffer, length);
	// Change permissions back 
	check = mprotect((void*)&calling_tpsb->tps[offset], length, PROT_NONE);
	if (check == -1)
		return -1; 
	exit_critical_section();
	return 0;
}

int tps_clone(pthread_t tid)
{
	 enter_critical_section();

	// call tps_create() to make new TPS, copy content from target thread's TPS w/memcpy()
	void* toBeClnd = NULL;
	void* wantsToCln = NULL;
	int check;
	
	// if getTPS() fails, thread with tid as argument has no TPSB or tps...
	check = getTPS(toBeClnd, tid);
	if (check == -1)
		return -1;
	tpsb_t toBeClndCstd = (tpsb_t)toBeClnd;

	// if getTPS() does not fail, calling thread alerady has TPSB with tps...
	check = getTPS(wantsToCln, NA);
	if (check != -1)
		return -1;

	tps_create();
	check = getTPS(wantsToCln, NA);
	if (check == -1)
		return -1;
	tpsb_t wantsToClnCstd = (tpsb_t)wantsToCln;

	memcpy((void*)wantsToClnCstd->tps, (const void*)toBeClndCstd->tps, TPS_SIZE);
	
	exit_critical_section();
	return 0;
}


