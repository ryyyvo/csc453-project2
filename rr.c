#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <math.h>

#include "lwp.h"

#define MAX_PROCESSES 500
#define MAX_ARGUMENTS 10


/*struct scheduler RR = {init, shutdown, *admit, *removeR, *next, *qlen};
scheduler curr_scheduler = &RR;
*/

unsigned long *queue;
int first=-1; /*ind of first proc*/
int back=-1; /*ind of last proc*/
int threads_in_queue = 0;

thread *thread_table;

void init(void){
    queue = malloc(MAX_PROCESSES * sizeof(unsigned long));
	thread_table = malloc(MAX_PROCESSES * sizeof(context));
}

void shutdown(void){
    free(queue);
}

int isFull()
{
    return (back + 1) % MAX_PROCESSES == first;
}

int isEmpty(){
    return first == -1;
}


void admit(thread new)
{
    /*unsigned long t_id = new.tid;
	*/

	unsigned long t_id = new->tid;
    if(isFull()){
        unsigned long *more_space = realloc(queue, 100*sizeof(unsigned long));
		thread *new_table = realloc(queue, 100*sizeof(context));

        if (more_space == NULL){
            printf("realloc failed");
            return;
        }
        queue = more_space;

		if(new_table == NULL){
			printf("Realloc failed");
			return;
		}
		thread_table = new_table;
    }

    if(first == -1){
        first = 0;
    }

    back = (back + 1) % MAX_PROCESSES;
    queue[back] = t_id;
	thread_table[t_id] = new;
    threads_in_queue++;
}

void removeR(thread victim){
    unsigned long t_id = victim->tid;
    if (isEmpty()){
        printf("queue empty");

    }

    if (first == back) {
        first = back = -1;
    }
    else{
        first = (first + 1) % MAX_PROCESSES;
    }
	free(thread_table[t_id]);
    threads_in_queue--;
}


thread next(){
    unsigned long t_id = queue[first];
    
    return thread_table[t_id];
}


thread tid2thread_helper(tid_t tid){
	return thread_table[tid];
}

int qlen(){
    return threads_in_queue;
}


struct scheduler RR = {init, shutdown, admit, removeR, next, qlen};
/*scheduler curr_scheduler = &RR;*/
