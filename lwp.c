#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include "include/lwp.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>
#include "rr.h"

int tid_counter = 1;
int stack_size;
struct rlimit rl_struct;
struct scheduler RS = RR;
thread current_thread = NULL; // keep track of current thread


int get_stack_size(){
/* finding stack size*/
	long p_size;
	p_size = sysconf(_SC_PAGE_SIZE);

	if(getrlimit(RLIMIT_STACK, &rl_struct) == 0 && rl_struct.rlim_cur != RLIM_INFINITY){
		stack_size = rl_struct.rlim_cur;
	} 
	else{
		stack_size = 8 * 1024 * 1024;
	}

	if(stack_size % p_size!= 0){
		stack_size += p_size - (stack_size%p_size);
	}
	return stack_size;
}


tid_t lwp_create(lwpfun function, void *argument){
	stack_size = get_stack_size();
	thread new_thread = malloc(sizeof(struct threadinfo_st));
	new_thread->tid = tid_counter;
	tid_counter++;
	
	int *args = (int *)argument;
	new_thread->state.rdi = args[0];
	new_thread->state.rsi = args[1];
	new_thread->state.rdx = args[2];
	new_thread->state.rcx = args[3];
	new_thread->state.r8 = args[4];	
	new_thread->state.r9 = args[5];
	new_thread->state.fxsave = FPU_INIT;
	new_thread->stacksize = stack_size;

	void *stack_pointer = mmap(NULL, stack_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0);
	new_thread->stack = (unsigned long*)((char*)stack_pointer + stack_size);
	/*new_thread->state.rsp = new_thread->stack;*/
	admit(new_thread);




	/*munmap(stack_pointer)*/	
	return new_thread->tid; 
}

void lwp_start(void) {
	thread main_thread = (thread) malloc(sizeof(struct threadinfo_st));
	if (main_thread == NULL) {
		perror("malloc");
		exit(1);
	}
	/* more to do!! */
	tid_counter++;
	main_thread->tid = tid_counter;
	main_thread->stack = NULL;
	lwp_yield();
}

void lwp_yield(void) {

}

void lwp_exit(int exitval);

tid_t lwp_wait(int *status) {
	if (&RS == NULL) { // is & right ?
		return NO_THREAD;
	}
	// ...
}

tid_t lwp_gettid(void) {
	if (current_thread == NULL) {
		return NO_THREAD;
	}
	return current_thread->tid;
}

thread tid2thread(tid_t tid) {
	// returns the thread correspodning to the given thread ID , or NULL if ID is invalid
	
	// while loop to find the thread with the given tid
	// if found return the thread
	// else return NULL
	return NULL;
}

void lwp_set_scheduler(scheduler sched);

scheduler lwp_get_scheduler(void) {
	return &RS; // is & right ? 
}

void main(int argc, char* argv){
	return;
}
