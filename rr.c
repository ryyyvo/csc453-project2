#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <math.h>

/*#include "./include/lwp.h"*/

#define MAX_PROCESSES 500
#define MAX_ARGUMENTS 10

typedef struct
{
    unsigned long tid;
} thread;

unsigned long queue[MAX_PROCESSES];

int first = -1; /*ind of first proc*/
int back = -1;  /*ind of last proc*/
int threads_in_queue = 0;

int isFull()
{
    return (back + 1) % MAX_PROCESSES == first;
}

int isEmpty()
{
    return first == -1;
}

void admit(thread new)
{
    unsigned long t_id = new.tid;
    if (isFull())
    {
        printf("queue full");
        return;
    }

    if (first == -1)
    {
        first = 0;
    }

    back = (back + 1) % MAX_PROCESSES;
    queue[back] = t_id;
    threads_in_queue++;
}

void removeR(thread victim)
{
    unsigned long t_id = victim.tid;
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
    threads_in_queue--;
}

thread next()
{
    unsigned long t_id = queue[first];
    /* tid2thread function called and returned */
    thread try = {123456UL};
    return try;
}

int qlen()
{
    return threads_in_queue;
}

int main(int argc, char *argv[])
{
    unsigned long id = 12345UL;
    thread try = {123456UL};
    admit(try);
    printf("%lu\n", queue[first]);
    thread try2 = {56789UL};
    admit(try2);
    thread n = next();
    printf("%lu\n", n.tid);
    removeR(n);
    printf("%lu\n", queue[first]);
}

struct scheduler RR = {NULL, NULL, *admit, *removeR, *next, *qlen};