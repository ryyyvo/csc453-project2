#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <math.h>

#include "./include/lwp.h"

int threads_in_list = 0;

struct Node
{
    context data;
    struct Node *next;
}

void
init(void)
{
    node = malloc(MAX_PROCESSES * sizeof(unsigned long));
    thread_table = malloc(MAX_PROCESSES * sizeof(context));
}

void shutdown(void)
{
    free(queue);
}

int isEmpty()
{
    return first == -1;
}

void admit(thread new)
{
    /*unsigned long t_id = new.tid;
     */

    unsigned long t_id = new->tid;
    if (isFull())
    {
        unsigned long *more_space = realloc(queue, 100 * sizeof(unsigned long));
        thread *new_table = realloc(queue, 100 * sizeof(context));

        if (more_space == NULL)
        {
            printf("realloc failed");
            return;
        }
        queue = more_space;

        if (new_table == NULL)
        {
            printf("Realloc failed");
            return;
        }
        thread_table = new_table;
    }

    if (first == -1)
    {
        first = 0;
    }

    back = (back + 1) % MAX_PROCESSES;
    queue[back] = t_id;
    thread_table[t_id] = new;
    threads_in_queue++;
}

void removeR(thread victim)
{
    unsigned long t_id = victim->tid;
    if (isEmpty())
    {
        printf("queue empty");
    }

    if (first == back)
    {
        first = back = -1;
    }
    else
    {
        first = (first + 1) % MAX_PROCESSES;
    }
    free(thread_table[t_id]);
    threads_in_queue--;
}

thread next()
{
    unsigned long t_id = queue[first];

    return thread_table[t_id];
}

thread tid2thread_helper(tid_t tid)
{
    return thread_table[tid];
}

int qlen()
{
    return threads_in_list;
}

void main(int argc, char *argv[])
{
    return;
}

struct scheduler RR = {init, shutdown, *admit, *removeR, *next, *qlen};
