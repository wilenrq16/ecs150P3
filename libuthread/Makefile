# Target library
lib := libuthread.a
CFLAGS := -Wall -Werror -g 
all: $(lib)

$(lib): queue.o thread.o sem.o #tps.o
	ar rcs $(lib) $^ -l 

sem.o: sem.c sem.h 
	gcc $(CFLAGS) $< -c 

##tps.o: tps.c tps.h 
##	gcc $(CFLAGS) $< -c 

clean: rm -rf $(lib) sem.o #tps.o 


