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
thread all_threads_head = NULL;  // linked list of ALL threads

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

    new_thread->lib_one = all_threads_head;
    new_thread->lib_two = NULL;
    if (all_threads_head) {
        all_threads_head->lib_two = new_thread;
    }
    all_threads_head = new_thread;

	RS->admit(new_thread);
	
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

    main_thread->lib_one = all_threads_head;
    main_thread->lib_two = NULL;
    if (all_threads_head) {
        all_threads_head->lib_two = main_thread;
    }
    all_threads_head = main_thread;

	RS->admit(main_thread);
	current_thread = main_thread;
	lwp_yield();
}

void lwp_yield(void)
{
    thread next_thread = RS->next();

    if (!next_thread) {
        while (all_threads_head) {
            thread t = all_threads_head;
            all_threads_head = all_threads_head->lib_one;
            
            if (t->stack != NULL) {
                void *stack_base = (void *)((unsigned long)t->stack - t->stacksize / 8);
                munmap(stack_base, t->stacksize);
            }
            free(t);
        }
        
        exit(3);
    }
    
    if (next_thread == current_thread) {
        return;
    }
    
    thread old_thread = current_thread;
    current_thread = next_thread;

    swap_rfiles(&old_thread->state, &next_thread->state);
}

void lwp_exit(int exitval) {
    if (!current_thread) {
        exit(exitval);
    }

    current_thread->status = MKTERMSTAT(LWP_TERM, exitval);

    RS->remove(current_thread);

    lwp_yield();
};

tid_t lwp_wait(int *status)
{
    if (RS->qlen() == 0) {
        return NO_THREAD;
    }
    
    thread terminated = NULL;
    
    while (!terminated) {
        thread t = all_threads_head;
        while (t) {
            if (LWPTERMINATED(t->status)) {
                terminated = t;
                break;
            }
            t = t->lib_one;
        }
        
        if (terminated) {
            break; 
        }
        
        if (RS->qlen() == 1 && current_thread) {
            return NO_THREAD;
        }

        lwp_yield();
    }
    

    if (terminated->lib_two) {
        terminated->lib_two->lib_one = terminated->lib_one;
    }
    if (terminated->lib_one) {
        terminated->lib_one->lib_two = terminated->lib_two;
    }
    if (all_threads_head == terminated) {
        all_threads_head = terminated->lib_one;
    }
    
    tid_t tid = terminated->tid;
    
    if (status) {
        *status = terminated->status;
    }
    
    if (terminated->stack != NULL) {
        void *stack_base = (void *)((unsigned long)terminated->stack - terminated->stacksize / 8);
        munmap(stack_base, terminated->stacksize);
    }
    
    free(terminated);
    
    return tid;
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

void lwp_set_scheduler(scheduler sched){
    thread temp;
    if(sched == NULL){
        return;
    }

    if(sched->init){
        sched->init();
    }

	temp = RS->next();
    while(temp != NULL){
        RS->remove(temp);
        sched->admit(temp);
    	temp = RS->next();
	}
    

    if(RS->shutdown){
        RS->shutdown();
    }

    RS = sched;

    return;
}
scheduler lwp_get_scheduler(void)
{
	return RS;
}
