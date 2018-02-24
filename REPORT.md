ECS 150 Operating Systems: Project 3

Joseph Torres
Vedant Ruparell

## Overview
The aim of this project is to investigate the way the access of multiple threads to 
shared objects can be *synchronized*, or managed, in the context of concurrency.
The project is divided into two sections. The first features an implementation of 
semaphores, a synchronization construct used reserve access to a shared object
by one thread at the mutual exclusion of the others. The second involves the 
creation of per-thread private storage (TPS) that would allow threads of the 
same process (that is, with access the same pool memory address space) to share
information safely. 


## Works Cited
In addition to the slides provided in 07.sync.pdf, 08.deadlocks.pdf *Operating
Systems: Principles and Practice* chapter 5, and the assignment specifications,
the following sources were used:

 - To assist in the use of mmap() to create private storage pages:
 http://man7.org/linux/man-pages/man2/mmap.2.html
 
 - To assist in the use of memcpy() to copy areas of memory:
 http://man7.org/linux/man-pages/man3/memcpy.3.html

 - To assist in the use of mprotect() to alter permissions:
 http://man7.org/linux/man-pages/man2/mprotect.2.html

 - To assist in the writing of a signal handler to deal with segmentation faults:
 http://pubs.opengroup.org/onlinepubs/7908799/xsh/sigaction.html

 - To assist in debugging:
 http://valgrind.org/docs/manual/quick-start.html
 
 - To read up on pointer arithmetic and the C programming language in general: 
 Forouzan, Behrouz A, and Richard F Gilberg. *Computer Science: A Structured
 Approach Using C*. 3rd ed., Thomas Course Technology, 2007.


## Testing
Several measures were taken to ensure faithfulness to the sem.c and tps.c 
specifications. They are the following:

 - Comparison with *OSPP*, Chapter Five, Figure 5.11
 - Analysis using gdb to verify the behavior of sem.c during its use in tests
 sem_count.x, sem_buffer.x, sem_prime.x.
 - Analysis of the gdb output of tps.x. 
 - Analysis of the valgrind output of tps.x to address resilient segmentation 
 faults in tps.c. 


## Implementation

### Phase 1:
Our semaphore API began with of with the design of a semaphore struct
containing the variables value and queue. The value variable refers to 0 or 1, 1
meaning it is ready to be acquired by a thread and 0 means it is currently being
used by a thread. Our first function was sem_create(size_t count) and was used
to create a semaphore by passing in the count as the current value, and 
initializing an empty queue for the threads that will wait to use the resource. 
Next came our sem_destroy(sem_t sem) that destroyed the queue of threads 
contained by the semaphore and then deallocated the necessary space. Then 
came the sem_up() and sem_down() functions that incremented or 
decremented the value of the semaphore, and then either enqueues and blocks
a thread or dequeues or unblocks a thread thread respectively.

### Phase 2:

#### 2.1
Our thread private storage (TPS) API started of with the TPSB struct containing 
variables tid of time pthread_t and tps of type char*. TPSB refers to a thread 
private storage block that contains a thread ID and an associative thread 
private storage memory page that belongs to the respective thread. We also 
created a pointer to the TPSB struct and named it tpsb_t and created a global 
variable library of type queue_t to hold the necessary TPSB objects. Every time
we needed to perform tasks on a certain thread, we would need to iterate the 
queue in order obtain the thread tid and then do the necessary and therefore we
created a function getTPS() to do so. First we designed the tps_init() function that 
checks if our library has been initialized and, if not, initialized it. Our 
tps_create() function created a memory page using mmap() and associated it with the
current thread by means of its tid as a TPSB object, and then we add it to our 
library queue to store all threads that have a private storage page. Then we made
our tps_destroy() function that deletes the necessary private memory page and 
subsequently, deletes the thread from the queue. Then came our read and write
functions which made use of the enter and exit critical section functions from 
the thread file. Upon iterating to the respecting thread, we called memcopy() 
with the buffer variable being the destination for read, and the source for write.
Lastly came the tps_clone(pthread_t tid) function that naive cloned the passed in
thread's TPS to a new thread that was being created. We had to check if the 
passed thread existed, as well as if the thread to be clones already existed. Then,
we simply used memcopy() to copy the given thread's TPS to the thread being 
created.

#### 2.2
Phase 2.2 builds on Phase 2.1, but rethinks the approach of liberally bestowing 
open read and write permissions on each and every TPS that is created. Thus, 
the mmap() used in tps_create now sets the permissions to PROT_NONE. In 
tps_read and tps_write, mprotect() is used to set the permission to PROT_READ 
and PROT_WRITE respectively just to the area in the tps page that will be read 
from or written too respectively. These permissions resume their PROT_NONE 
state after the read or write takes place. These functions uses similar syntax to 
the memcpy() used in the same function, and the variable check is recycled for 
the same success-ensuring purpose. Meanwhile, tps_init adopts a signal handler 
activate upon segmentation faults(SIGSEGV), or attempting invalid access at 
valid memory, and bus errors (SIGBUS), or validly attempting access at an invalid
address. The tpsb libarary is consulted to check whether the event was caused 
by an attempt to access a protected page or by some other unrelated cause. 

#### 2.3
We were unable to complete Phase 2.3. 

