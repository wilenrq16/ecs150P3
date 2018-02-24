ECS150 P3 Write-Up
==========
Joseph Torres   
Vedant Ruparell  

### Introduction
The aim of the following project was to extend our understanding of threads. The project was divided into two independent sections: the first being an implementation of semaphores to control and synchronize common resources by multiple threads, and the second being a thread private storage (TPS) that would allow threads of the same process to easily share information as they all share the same memory address space.
      

### Our Implementation
##### Phase 1: 
Our semaphore API started of with the semaphore struct, containing variables: value and queue. The value variable refers to 0 or 1, 1 meaning it is ready to be acquired by a thread and 0 means it is currently being used by a thread.  
Our first function was sem_create(size_t count) and was used to create a semaphore by passing in the count as the current value, and initializing an empty queue for the threads that will await to use the resource.  
Next came our sem_destroy(sem_t sem) that destroyed the queue of threads contained by the semaphore and then deallocated the necessary space.  
Then came the sem_up() and sem_down() functions that incremented or decremented the value of the semaphore, and then either enqueues and blocks a thread or dequeues or unblocks a thread thread respectively. 

##### Phase 2:
###### 2.1
Our thread private storage (TPS) API started of with the TPSB struct containing variables tid of time pthread_t and tps of type char*. TPSB refers to a thread private storage block that contains a thread ID and an associative thread private storage memory page that belongs to the respective thread. We also created a pointer to the TPSB struct and named it tpsb_t and created a global variable library of type queue_t to hold the necessary TPSB objects. Every time we need to perform tasks on a certain thread, we would need to iterate the queue in order obtain the thread tid and then do the necessary and therefore we created a function getTPS() to do so. 
First came the tps_init() function that checks if our library has been initialized and if it hasn't, we initialize it.  
Our tps_create() function created a memory page using mmap() and associated it with the current thread by means of its tid as a TPSB object, and then we add it to our library queue to store all threads that have a private storage page.  
Next came our tps_destroy() function that deletes the necessary private memory page and subsequently, deletes the thread from the queue.  
Then came our read and write functions which made use of the enter and exit critical section functions from the thread file. Upon iterating to the respecting thread, we called memcopy() with the buffer variable being the destination for read, and the source for write.
Lastly came the tps_clone(pthread_t tid) function that naive cloned the passed in thread's TPS to a new thread that was being created. We had to check if the passed thread existed, as well as if the thread to be clones already existed. Then, we simply used memcopy() to copy the given thread's TPS to the thread being created.
###### 2.2
Did not complete.

###### 2.3
Did not complete


#### Conclusion and Closing Remarks
With the amount we accomplished, we felt that we succeeded in extending our understanding of threads.   
Our lack of time management prevented us from completing the necessary tasks. This is something we will take with us to the next project.
