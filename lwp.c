#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include "lwp.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>
#include "rr.h"

int tid_counter = 1;
int stack_size;
struct rlimit rl_struct;
scheduler RS = &RR;
thread current_thread = NULL; // keep track of current thread

int get_stack_size()
{
	/* finding stack size*/
	long p_size;
	p_size = sysconf(_SC_PAGE_SIZE);

	if (getrlimit(RLIMIT_STACK, &rl_struct) == 0 && rl_struct.rlim_cur != RLIM_INFINITY)
	{
		stack_size = rl_struct.rlim_cur;
	}
	else
	{
		stack_size = 8 * 1024 * 1024;
	}

	if (stack_size % p_size != 0)
	{
		stack_size += p_size - (stack_size % p_size);
	}
	return stack_size;
}

static void lwp_wrap(lwpfun fun, void *arg)
{
	int rval;
	rval = fun(arg);
	lwp_exit(rval);
}

tid_t lwp_create(lwpfun function, void *argument)
{
	stack_size = get_stack_size();
	thread new_thread = malloc(sizeof(struct threadinfo_st));
	new_thread->tid = tid_counter;
	tid_counter++;

	new_thread->state.rdi = (unsigned long)function;
	new_thread->state.rsi = (unsigned long)argument;

	new_thread->state.fxsave = FPU_INIT;
	new_thread->stacksize = stack_size;

	void *stack_pointer = mmap(NULL, stack_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);

	unsigned long base_add = (unsigned long)(char *)(stack_pointer + stack_size / 8);
	/*check if dividible by 16*/
	if (base_add % 16 != 0)
	{
		base_add -= base_add % 16;
	}

	base_add--;
	new_thread->stack = (unsigned long *)base_add;

	unsigned long *base_add_pointer = (unsigned long *)base_add;

	base_add_pointer--;

	*(base_add_pointer) = (unsigned long)lwp_wrap;
	base_add_pointer--;

	new_thread->state.rsp = (unsigned long)(base_add_pointer);
	new_thread->state.rbp = new_thread->state.rsp;

	/*
	 * stack looks like:
	 *
	 * lwp_wrap
	 * old rbp
	 * return address
	 * base of stack(TOS)
	 *
	 *
	 *
	 * */

	/*RS->admit(new_thread);
	 */
	swap_rfiles(NULL, &new_thread->state);
	/*munmap(stack_pointer), prob in exit*/
	return new_thread->tid;
}

void lwp_start(void)
{
	thread main_thread = (thread)malloc(sizeof(struct threadinfo_st));
	if (!main_thread)
	{
		perror("malloc");
		exit(1);
	}
	main_thread->tid = tid_counter;
	main_thread->stack = NULL;
	tid_counter++;
	RS->admit(main_thread); // admit the thread to the scheduler
	current_thread = main_thread;
	lwp_yield();
}

void lwp_yield(void)
{
}

void lwp_exit(int exitval) {
};

tid_t lwp_wait(int *status)
{
	if (!&RS)
	{ // is & right ?
		return NO_THREAD;
	}
	// ...
}

tid_t lwp_gettid(void)
{
	if (!current_thread)
	{
		return NO_THREAD;
	}
	return current_thread->tid;
}

thread tid2thread(tid_t tid)
{
	// returns the thread corresponding to the given thread ID , or NULL if ID is invalid

	if (!RS || RS->qlen() == 0)
	{
		return NULL;
	}

	thread t = RS->next();
	thread start = t;

	if (t->tid == tid)
	{
		return t;
	}

	t = RS->next();
	while (t != start && t != NULL)
	{
		if (t->tid == tid)
		{
			return t;
		}
		t = RS->next();
	}
	return NULL;
}

void lwp_set_scheduler(scheduler sched);

scheduler lwp_get_scheduler(void)
{
	return &RS; // is & right ?
}
