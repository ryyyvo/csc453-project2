#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include "include/lwp.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>


int tid_counter = 1;
int stack_size;
long p_size = sysconf(_SC_PAGE_SIZE);
struct rlimit rl_struct;

/* finding stack size*/
if(getrlimit(RLIMIT_STACK, &rl_struct) == 0 && rl_struct.rlim_cur != RLIM_INFINITY){
	stack_size = rl_struct.rlim_cur;
} 
else{
	stack_size = 8 * 1024 * 1024;
}

if(stack_size % p_size!= 0){
	stack_size += p_size - (stack_size%p_size);
}

tid_t lwp_create(lwpfun function, void *argument){
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
	new_thread->stacksize = STACK_SIZE;

	void *stack_pointer = mmap(NULL, stack_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0);
	new_thread->stack = (unsigned long*)((char*)stack_pointer + stack_size);
	new_thread->rsp = new_thread->stack;
	admit(new_thread);




	/*munmap(stack_pointer)*/	
	return new_thread->tid; 
}

void main(int argc, char* argv){
	return;
}
